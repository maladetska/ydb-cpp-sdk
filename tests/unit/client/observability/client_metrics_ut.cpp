#include <src/client/impl/observability/client_metrics.h>
#include <src/client/query/impl/query_metrics.h>
#include <src/client/table/impl/table_metrics.h>
#include <tests/common/fake_metric_registry.h>
#include <util/string/cast.h>

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
        return Registry->GetCounter("db.client.operation.requests", {
            {"db.system.name", "other_sql"},
            {"db.operation.name", op},
        });
    }

    std::shared_ptr<TFakeCounter> ErrorCounter(const std::string& op) {
        return Registry->GetCounter("db.client.operation.errors", {
            {"db.system.name", "other_sql"},
            {"db.operation.name", op},
        });
    }

    std::shared_ptr<TFakeHistogram> DurationHistogram(const std::string& op, EStatus status) {
        TLabels labels = {
            {"db.system.name", "other_sql"},
            {"db.operation.name", op},
            {"db.response.status_code", ToString(status)},
        };
        if (status != EStatus::SUCCESS) {
            labels["error.type"] = ToString(status);
        }
        return Registry->GetHistogram("db.client.operation.duration", labels);
    }

    std::shared_ptr<TFakeMetricRegistry> Registry;
};

TEST_F(ClientMetricsTest, RequestCounterIncrementedOnConstruction) {
    TClientMetrics metrics(Registry, "DoSomething");

    auto counter = RequestCounter("DoSomething");
    ASSERT_NE(counter, nullptr);
    EXPECT_EQ(counter->Get(), 1);
}

TEST_F(ClientMetricsTest, SuccessDoesNotIncrementErrorCounter) {
    {
        TClientMetrics metrics(Registry, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    }

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);
}

TEST_F(ClientMetricsTest, FailureIncrementsErrorCounter) {
    {
        TClientMetrics metrics(Registry, "DoSomething");
        metrics.End(EStatus::UNAVAILABLE);
    }

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 1);
}

TEST_F(ClientMetricsTest, DurationRecordedOnEnd) {
    {
        TClientMetrics metrics(Registry, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    }

    auto hist = DurationHistogram("DoSomething", EStatus::SUCCESS);
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
    EXPECT_GE(hist->GetValues()[0], 0.0);
}

TEST_F(ClientMetricsTest, DurationIsInSeconds) {
    {
        TClientMetrics metrics(Registry, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    }

    auto hist = DurationHistogram("DoSomething", EStatus::SUCCESS);
    ASSERT_NE(hist, nullptr);
    EXPECT_LT(hist->GetValues()[0], 1.0);
}

TEST_F(ClientMetricsTest, DoubleEndIsIdempotent) {
    TClientMetrics metrics(Registry, "DoSomething");
    metrics.End(EStatus::SUCCESS);
    metrics.End(EStatus::INTERNAL_ERROR);

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto hist = DurationHistogram("DoSomething", EStatus::SUCCESS);
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
}

TEST_F(ClientMetricsTest, DestructorCallsEndWithClientInternalError) {
    {
        TClientMetrics metrics(Registry, "DoSomething");
    }

    auto requests = RequestCounter("DoSomething");
    ASSERT_NE(requests, nullptr);
    EXPECT_EQ(requests->Get(), 1);

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 1);

    auto hist = DurationHistogram("DoSomething", EStatus::CLIENT_INTERNAL_ERROR);
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
}

TEST_F(ClientMetricsTest, NullRegistryDoesNotCrash) {
    EXPECT_NO_THROW({
        TClientMetrics metrics(nullptr, "DoSomething");
        metrics.End(EStatus::SUCCESS);
    });
}

TEST_F(ClientMetricsTest, DifferentOperationsHaveSeparateMetrics) {
    {
        TClientMetrics m1(Registry, "OpA");
        m1.End(EStatus::SUCCESS);
    }
    {
        TClientMetrics m2(Registry, "OpB");
        m2.End(EStatus::OVERLOADED);
    }

    EXPECT_EQ(RequestCounter("OpA")->Get(), 1);
    EXPECT_EQ(RequestCounter("OpB")->Get(), 1);
    EXPECT_EQ(ErrorCounter("OpA")->Get(), 0);
    EXPECT_EQ(ErrorCounter("OpB")->Get(), 1);
    EXPECT_EQ(DurationHistogram("OpA", EStatus::SUCCESS)->Count(), 1u);
    EXPECT_EQ(DurationHistogram("OpB", EStatus::OVERLOADED)->Count(), 1u);
}

TEST_F(ClientMetricsTest, MultipleRequestsAccumulate) {
    for (int i = 0; i < 5; ++i) {
        TClientMetrics metrics(Registry, "Op");
        metrics.End(i % 2 == 0 ? EStatus::SUCCESS : EStatus::TIMEOUT);
    }

    EXPECT_EQ(RequestCounter("Op")->Get(), 5);
    EXPECT_EQ(ErrorCounter("Op")->Get(), 2);
    EXPECT_EQ(DurationHistogram("Op", EStatus::SUCCESS)->Count(), 3u);
    EXPECT_EQ(DurationHistogram("Op", EStatus::TIMEOUT)->Count(), 2u);
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
        TClientMetrics metrics(Registry, "Op");
        metrics.End(status);
    }

    auto errors = ErrorCounter("Op");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), static_cast<int64_t>(errorStatuses.size()));
}

// ---------------------------------------------------------------------------
// TQueryMetrics
// ---------------------------------------------------------------------------

TEST(QueryMetricsTest, UsesOtelStandardMetrics) {
    auto registry = std::make_shared<TFakeMetricRegistry>();

    NQuery::TQueryMetrics metrics(registry, "ExecuteQuery");
    metrics.End(EStatus::SUCCESS);

    EXPECT_NE(
        registry->GetCounter(
            "db.client.operation.requests",
            {{"db.system.name", "other_sql"}, {"db.operation.name", "ExecuteQuery"}}
        ),
        nullptr
    );
    EXPECT_NE(
        registry->GetCounter(
            "db.client.operation.errors",
            {{"db.system.name", "other_sql"}, {"db.operation.name", "ExecuteQuery"}}
        ),
        nullptr
    );
    EXPECT_NE(
        registry->GetHistogram(
            "db.client.operation.duration",
            {
                {"db.system.name", "other_sql"},
                {"db.operation.name", "ExecuteQuery"},
                {"db.response.status_code", ToString(EStatus::SUCCESS)},
            }
        ),
        nullptr
    );
}

// ---------------------------------------------------------------------------
// TTableMetrics
// ---------------------------------------------------------------------------

TEST(TableMetricsTest, UsesOtelStandardMetrics) {
    auto registry = std::make_shared<TFakeMetricRegistry>();

    NTable::TTableMetrics metrics(registry, "ExecuteDataQuery");
    metrics.End(EStatus::SUCCESS);

    EXPECT_NE(
        registry->GetCounter(
            "db.client.operation.requests",
            {{"db.system.name", "other_sql"}, {"db.operation.name", "ExecuteDataQuery"}}
        ),
        nullptr
    );
    EXPECT_NE(
        registry->GetCounter(
            "db.client.operation.errors",
            {{"db.system.name", "other_sql"}, {"db.operation.name", "ExecuteDataQuery"}}
        ),
        nullptr
    );
    EXPECT_NE(
        registry->GetHistogram(
            "db.client.operation.duration",
            {
                {"db.system.name", "other_sql"},
                {"db.operation.name", "ExecuteDataQuery"},
                {"db.response.status_code", ToString(EStatus::SUCCESS)},
            }
        ),
        nullptr
    );
}
