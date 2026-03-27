#include <src/client/impl/observability/client_metrics.h>
#include <src/client/query/impl/query_metrics.h>
#include <src/client/table/impl/table_metrics.h>
#include <tests/common/fake_metric_registry.h>

#include <gtest/gtest.h>

using namespace NYdb;
using namespace NYdb::NObservability;
using namespace NYdb::NMetrics;
using namespace NYdb::NTests;

// ---------------------------------------------------------------------------
// TClientMetrics (shared logic)
// ---------------------------------------------------------------------------

class ClientMetricsTest : public ::testing::Test {
protected:
    void SetUp() override {
        Registry = std::make_shared<TFakeMetricRegistry>();
    }

    std::shared_ptr<TFakeCounter> RequestCounter(const std::string& op) {
        return Registry->GetCounter(Prefix + ".requests", {{"operation", op}});
    }

    std::shared_ptr<TFakeCounter> ErrorCounter(const std::string& op) {
        return Registry->GetCounter(Prefix + ".errors", {{"operation", op}});
    }

    std::shared_ptr<TFakeHistogram> DurationHistogram(const std::string& op) {
        return Registry->GetHistogram("db.client.operation.duration", {
            {"db.system.name", "ydb"},
            {"db.operation.name", op},
        });
    }

    const std::string Prefix = "ydb.test";
    std::shared_ptr<TFakeMetricRegistry> Registry;
};

TEST_F(ClientMetricsTest, RequestCounterIncrementedOnConstruction) {
    TClientMetrics metrics(Registry, Prefix, "DoSomething");

    auto counter = RequestCounter("DoSomething");
    ASSERT_NE(counter, nullptr);
    EXPECT_EQ(counter->Get(), 1);
}

TEST_F(ClientMetricsTest, SuccessDoesNotIncrementErrorCounter) {
    {
        TClientMetrics metrics(Registry, Prefix, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    }

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);
}

TEST_F(ClientMetricsTest, FailureIncrementsErrorCounter) {
    {
        TClientMetrics metrics(Registry, Prefix, "DoSomething");
        metrics.End(EStatus::UNAVAILABLE);
    }

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 1);
}

TEST_F(ClientMetricsTest, DurationRecordedOnEnd) {
    {
        TClientMetrics metrics(Registry, Prefix, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    }

    auto hist = DurationHistogram("DoSomething");
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
    EXPECT_GE(hist->GetValues()[0], 0.0);
}

TEST_F(ClientMetricsTest, DurationIsInSeconds) {
    {
        TClientMetrics metrics(Registry, Prefix, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    }

    auto hist = DurationHistogram("DoSomething");
    ASSERT_NE(hist, nullptr);
    EXPECT_LT(hist->GetValues()[0], 1.0);
}

TEST_F(ClientMetricsTest, DoubleEndIsIdempotent) {
    TClientMetrics metrics(Registry, Prefix, "DoSomething");
    metrics.End(EStatus::SUCCESS);
    metrics.End(EStatus::INTERNAL_ERROR);

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto hist = DurationHistogram("DoSomething");
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
}

TEST_F(ClientMetricsTest, DestructorCallsEndWithClientInternalError) {
    {
        TClientMetrics metrics(Registry, Prefix, "DoSomething");
    }

    auto requests = RequestCounter("DoSomething");
    ASSERT_NE(requests, nullptr);
    EXPECT_EQ(requests->Get(), 1);

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 1);

    auto hist = DurationHistogram("DoSomething");
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
}

TEST_F(ClientMetricsTest, NullRegistryDoesNotCrash) {
    EXPECT_NO_THROW({
        TClientMetrics metrics(nullptr, Prefix, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    });
}

TEST_F(ClientMetricsTest, DifferentOperationsHaveSeparateMetrics) {
    {
        TClientMetrics m1(Registry, Prefix, "OpA");
        m1.End(EStatus::SUCCESS);
    }
    {
        TClientMetrics m2(Registry, Prefix, "OpB");
        m2.End(EStatus::OVERLOADED);
    }

    EXPECT_EQ(RequestCounter("OpA")->Get(), 1);
    EXPECT_EQ(RequestCounter("OpB")->Get(), 1);
    EXPECT_EQ(ErrorCounter("OpA")->Get(), 0);
    EXPECT_EQ(ErrorCounter("OpB")->Get(), 1);
    EXPECT_EQ(DurationHistogram("OpA")->Count(), 1u);
    EXPECT_EQ(DurationHistogram("OpB")->Count(), 1u);
}

TEST_F(ClientMetricsTest, MultipleRequestsAccumulate) {
    for (int i = 0; i < 5; ++i) {
        TClientMetrics metrics(Registry, Prefix, "Op");
        metrics.End(i % 2 == 0 ? EStatus::SUCCESS : EStatus::TIMEOUT);
    }

    EXPECT_EQ(RequestCounter("Op")->Get(), 5);
    EXPECT_EQ(ErrorCounter("Op")->Get(), 2);
    EXPECT_EQ(DurationHistogram("Op")->Count(), 5u);
}

TEST_F(ClientMetricsTest, AllErrorStatusesIncrementErrorCounter) {
    std::vector<EStatus> errorStatuses = {
        EStatus::BAD_REQUEST,
        EStatus::UNAUTHORIZED,
        EStatus::INTERNAL_ERROR,
        EStatus::UNAVAILABLE,
        EStatus::OVERLOADED,
        EStatus::TIMEOUT,
        EStatus::NOT_FOUND,
        EStatus::CLIENT_INTERNAL_ERROR,
    };

    for (auto status : errorStatuses) {
        TClientMetrics metrics(Registry, Prefix, "Op");
        metrics.End(status);
    }

    auto errors = ErrorCounter("Op");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), static_cast<int64_t>(errorStatuses.size()));
}

TEST_F(ClientMetricsTest, PrefixAppliedToCounterNames) {
    TClientMetrics metrics(Registry, "ydb.custom", "Op");
    metrics.End(EStatus::SUCCESS);

    EXPECT_NE(Registry->GetCounter("ydb.custom.requests", {{"operation", "Op"}}), nullptr);
    EXPECT_NE(Registry->GetCounter("ydb.custom.errors", {{"operation", "Op"}}), nullptr);

    EXPECT_EQ(Registry->GetCounter("ydb.test.requests", {{"operation", "Op"}}), nullptr);
}

// ---------------------------------------------------------------------------
// TQueryMetrics prefix
// ---------------------------------------------------------------------------

TEST(QueryMetricsTest, UsesQueryPrefix) {
    auto registry = std::make_shared<TFakeMetricRegistry>();

    NQuery::TQueryMetrics metrics(registry, "ExecuteQuery");
    metrics.End(EStatus::SUCCESS);

    EXPECT_NE(registry->GetCounter("ydb.query.requests", {{"operation", "ExecuteQuery"}}), nullptr);
    EXPECT_NE(registry->GetCounter("ydb.query.errors", {{"operation", "ExecuteQuery"}}), nullptr);
    EXPECT_NE(registry->GetHistogram("db.client.operation.duration", {
        {"db.system.name", "ydb"}, {"db.operation.name", "ExecuteQuery"}}), nullptr);

    EXPECT_EQ(registry->GetCounter("ydb.table.requests", {{"operation", "ExecuteQuery"}}), nullptr);
}

// ---------------------------------------------------------------------------
// TTableMetrics prefix
// ---------------------------------------------------------------------------

TEST(TableMetricsTest, UsesTablePrefix) {
    auto registry = std::make_shared<TFakeMetricRegistry>();

    NTable::TTableMetrics metrics(registry, "ExecuteDataQuery");
    metrics.End(EStatus::SUCCESS);

    EXPECT_NE(registry->GetCounter("ydb.table.requests", {{"operation", "ExecuteDataQuery"}}), nullptr);
    EXPECT_NE(registry->GetCounter("ydb.table.errors", {{"operation", "ExecuteDataQuery"}}), nullptr);
    EXPECT_NE(registry->GetHistogram("db.client.operation.duration", {
        {"db.system.name", "ydb"}, {"db.operation.name", "ExecuteDataQuery"}}), nullptr);

    EXPECT_EQ(registry->GetCounter("ydb.query.requests", {{"operation", "ExecuteDataQuery"}}), nullptr);
}
