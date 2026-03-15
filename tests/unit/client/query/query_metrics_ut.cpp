#include <src/client/query/impl/query_metrics.h>
#include <tests/common/fake_metric_registry.h>

#include <gtest/gtest.h>

using namespace NYdb;
using namespace NYdb::NQuery;
using namespace NYdb::NMetrics;
using namespace NYdb::NTests;

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

class QueryMetricsTest : public ::testing::Test {
protected:
    void SetUp() override {
        Registry = std::make_shared<TFakeMetricRegistry>();
    }

    std::shared_ptr<TFakeCounter> RequestCounter(const std::string& op) {
        return Registry->GetCounter("ydb.query.requests", {{"operation", op}});
    }

    std::shared_ptr<TFakeCounter> ErrorCounter(const std::string& op) {
        return Registry->GetCounter("ydb.query.errors", {{"operation", op}});
    }

    std::shared_ptr<TFakeHistogram> LatencyHistogram(const std::string& op) {
        return Registry->GetHistogram("ydb.query.latency_ms", {{"operation", op}});
    }

    std::shared_ptr<TFakeMetricRegistry> Registry;
};

TEST_F(QueryMetricsTest, RequestCounterIncrementedOnConstruction) {
    TQueryMetrics metrics(Registry, "ExecuteQuery");

    auto counter = RequestCounter("ExecuteQuery");
    ASSERT_NE(counter, nullptr);
    EXPECT_EQ(counter->Get(), 1);
}

TEST_F(QueryMetricsTest, SuccessDoesNotIncrementErrorCounter) {
    {
        TQueryMetrics metrics(Registry, "ExecuteQuery");
        metrics.End(EStatus::SUCCESS);
    }

    auto errors = ErrorCounter("ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);
}

TEST_F(QueryMetricsTest, FailureIncrementsErrorCounter) {
    {
        TQueryMetrics metrics(Registry, "Commit");
        metrics.End(EStatus::UNAVAILABLE);
    }

    auto errors = ErrorCounter("Commit");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 1);
}

TEST_F(QueryMetricsTest, LatencyRecordedOnEnd) {
    {
        TQueryMetrics metrics(Registry, "Rollback");
        metrics.End(EStatus::SUCCESS);
    }

    auto hist = LatencyHistogram("Rollback");
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
    EXPECT_GE(hist->GetValues()[0], 0.0);
}

TEST_F(QueryMetricsTest, DoubleEndIsIdempotent) {
    TQueryMetrics metrics(Registry, "ExecuteQuery");
    metrics.End(EStatus::SUCCESS);
    metrics.End(EStatus::INTERNAL_ERROR);

    auto errors = ErrorCounter("ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto hist = LatencyHistogram("ExecuteQuery");
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
}

TEST_F(QueryMetricsTest, DestructorCallsEndWithClientInternalError) {
    {
        TQueryMetrics metrics(Registry, "CreateSession");
    }

    auto requests = RequestCounter("CreateSession");
    ASSERT_NE(requests, nullptr);
    EXPECT_EQ(requests->Get(), 1);

    auto errors = ErrorCounter("CreateSession");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 1);

    auto hist = LatencyHistogram("CreateSession");
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
}

TEST_F(QueryMetricsTest, NullRegistryDoesNotCrash) {
    EXPECT_NO_THROW({
        TQueryMetrics metrics(nullptr, "ExecuteQuery");
        metrics.End(EStatus::SUCCESS);
    });
}

TEST_F(QueryMetricsTest, CorrectMetricNamesAndLabels) {
    TQueryMetrics metrics(Registry, "ExecuteQuery");
    metrics.End(EStatus::SUCCESS);

    EXPECT_NE(Registry->GetCounter("ydb.query.requests", {{"operation", "ExecuteQuery"}}), nullptr);
    EXPECT_NE(Registry->GetCounter("ydb.query.errors", {{"operation", "ExecuteQuery"}}), nullptr);
    EXPECT_NE(Registry->GetHistogram("ydb.query.latency_ms", {{"operation", "ExecuteQuery"}}), nullptr);

    EXPECT_EQ(Registry->GetCounter("ydb.query.requests", {{"operation", "Commit"}}), nullptr);
}

TEST_F(QueryMetricsTest, DifferentOperationsHaveSeparateMetrics) {
    {
        TQueryMetrics m1(Registry, "ExecuteQuery");
        m1.End(EStatus::SUCCESS);
    }
    {
        TQueryMetrics m2(Registry, "Commit");
        m2.End(EStatus::OVERLOADED);
    }

    auto execRequests = RequestCounter("ExecuteQuery");
    auto commitRequests = RequestCounter("Commit");
    ASSERT_NE(execRequests, nullptr);
    ASSERT_NE(commitRequests, nullptr);
    EXPECT_EQ(execRequests->Get(), 1);
    EXPECT_EQ(commitRequests->Get(), 1);

    auto execErrors = ErrorCounter("ExecuteQuery");
    auto commitErrors = ErrorCounter("Commit");
    EXPECT_EQ(execErrors->Get(), 0);
    EXPECT_EQ(commitErrors->Get(), 1);
}

TEST_F(QueryMetricsTest, MultipleRequestsAccumulate) {
    for (int i = 0; i < 5; ++i) {
        TQueryMetrics metrics(Registry, "ExecuteQuery");
        metrics.End(i % 2 == 0 ? EStatus::SUCCESS : EStatus::TIMEOUT);
    }

    auto requests = RequestCounter("ExecuteQuery");
    ASSERT_NE(requests, nullptr);
    EXPECT_EQ(requests->Get(), 5);

    auto errors = ErrorCounter("ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 2);

    auto hist = LatencyHistogram("ExecuteQuery");
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 5u);
}

TEST_F(QueryMetricsTest, AllErrorStatusesIncrementErrorCounter) {
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
        TQueryMetrics metrics(Registry, "Rollback");
        metrics.End(status);
    }

    auto errors = ErrorCounter("Rollback");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), static_cast<int64_t>(errorStatuses.size()));
}
