#include <src/client/impl/observability/operation_metrics.h>
#include <src/client/impl/stats/stats.h>
#include <tests/common/fake_metric_registry.h>
#include <util/string/cast.h>

#include <gtest/gtest.h>

using namespace NYdb;
using namespace NYdb::NObservability;
using namespace NYdb::NMetrics;
using namespace NYdb::NTests;
using namespace NYdb::NSdkStats;

// ---------------------------------------------------------------------------
// TOperationMetrics (shared logic)
// ---------------------------------------------------------------------------

class OperationMetricsTest : public ::testing::Test {
protected:
    void SetUp() override {
        Registry = std::make_shared<TFakeMetricRegistry>();
        OpCollector.SetExternalRegistry(Registry);
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

    TStatCollector::TClientOperationStatCollector OpCollector;
    std::shared_ptr<TFakeMetricRegistry> Registry;
};

TEST_F(OperationMetricsTest, RequestCounterIncrementedOnConstruction) {
    TOperationMetrics metrics(&OpCollector, "DoSomething", TLog());

    auto counter = RequestCounter("DoSomething");
    ASSERT_NE(counter, nullptr);
    EXPECT_EQ(counter->Get(), 1);
}

TEST_F(OperationMetricsTest, SuccessDoesNotIncrementErrorCounter) {
    {
        TOperationMetrics metrics(&OpCollector, "DoSomething", TLog());
        metrics.End(EStatus::SUCCESS);
    }

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);
}

TEST_F(OperationMetricsTest, FailureIncrementsErrorCounter) {
    {
        TOperationMetrics metrics(&OpCollector, "DoSomething", TLog());
        metrics.End(EStatus::UNAVAILABLE);
    }

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 1);
}

TEST_F(OperationMetricsTest, DurationRecordedOnEnd) {
    {
        TOperationMetrics metrics(&OpCollector, "DoSomething", TLog());
        metrics.End(EStatus::SUCCESS);
    }

    auto hist = DurationHistogram("DoSomething", EStatus::SUCCESS);
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
    EXPECT_GE(hist->GetValues()[0], 0.0);
}

TEST_F(OperationMetricsTest, DurationIsInSeconds) {
    {
        TOperationMetrics metrics(&OpCollector, "DoSomething", TLog());
        metrics.End(EStatus::SUCCESS);
    }

    auto hist = DurationHistogram("DoSomething", EStatus::SUCCESS);
    ASSERT_NE(hist, nullptr);
    EXPECT_LT(hist->GetValues()[0], 1.0);
}

TEST_F(OperationMetricsTest, DoubleEndIsIdempotent) {
    TOperationMetrics metrics(&OpCollector, "DoSomething", TLog());
    metrics.End(EStatus::SUCCESS);
    metrics.End(EStatus::INTERNAL_ERROR);

    auto errors = ErrorCounter("DoSomething");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto hist = DurationHistogram("DoSomething", EStatus::SUCCESS);
    ASSERT_NE(hist, nullptr);
    EXPECT_EQ(hist->Count(), 1u);
}

TEST_F(OperationMetricsTest, DestructorCallsEndWithClientInternalError) {
    {
        TOperationMetrics metrics(&OpCollector, "DoSomething", TLog());
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

TEST_F(OperationMetricsTest, NullRegistryDoesNotCrash) {
    EXPECT_NO_THROW({
        TStatCollector::TClientOperationStatCollector nullCollector;
        TOperationMetrics metrics(&nullCollector, "DoSomething", TLog());
        metrics.End(EStatus::SUCCESS);
    });
}

TEST_F(OperationMetricsTest, DifferentOperationsHaveSeparateMetrics) {
    {
        TOperationMetrics m1(&OpCollector, "OpA", TLog());
        m1.End(EStatus::SUCCESS);
    }
    {
        TOperationMetrics m2(&OpCollector, "OpB", TLog());
        m2.End(EStatus::OVERLOADED);
    }

    EXPECT_EQ(RequestCounter("OpA")->Get(), 1);
    EXPECT_EQ(RequestCounter("OpB")->Get(), 1);
    EXPECT_EQ(ErrorCounter("OpA")->Get(), 0);
    EXPECT_EQ(ErrorCounter("OpB")->Get(), 1);
    EXPECT_EQ(DurationHistogram("OpA", EStatus::SUCCESS)->Count(), 1u);
    EXPECT_EQ(DurationHistogram("OpB", EStatus::OVERLOADED)->Count(), 1u);
}

TEST_F(OperationMetricsTest, MultipleRequestsAccumulate) {
    for (int i = 0; i < 5; ++i) {
        TOperationMetrics metrics(&OpCollector, "Op", TLog());
        metrics.End(i % 2 == 0 ? EStatus::SUCCESS : EStatus::TIMEOUT);
    }

    EXPECT_EQ(RequestCounter("Op")->Get(), 5);
    EXPECT_EQ(ErrorCounter("Op")->Get(), 2);
    EXPECT_EQ(DurationHistogram("Op", EStatus::SUCCESS)->Count(), 3u);
    EXPECT_EQ(DurationHistogram("Op", EStatus::TIMEOUT)->Count(), 2u);
}

TEST_F(OperationMetricsTest, AllErrorStatusesIncrementErrorCounter) {
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
        TOperationMetrics metrics(&OpCollector, "Op", TLog());
        metrics.End(status);
    }

    auto errors = ErrorCounter("Op");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), static_cast<int64_t>(errorStatuses.size()));
}

TEST(OperationMetricsAliasesTest, QueryOperationsUseOtelStandardMetrics) {
    auto registry = std::make_shared<TFakeMetricRegistry>();
    TStatCollector::TClientOperationStatCollector collector;
    collector.SetExternalRegistry(registry);

    NObservability::TOperationMetrics metrics(&collector, "ExecuteQuery", TLog());
    metrics.End(EStatus::SUCCESS);

    EXPECT_NE(
        registry->GetCounter(
            "db.client.operation.requests",
            {
                {"db.system.name", "other_sql"},
                {"db.operation.name", "ExecuteQuery"}
            }
        ),
        nullptr
    );
    EXPECT_NE(
        registry->GetCounter(
            "db.client.operation.errors",
            {
                {"db.system.name", "other_sql"},
                {"db.operation.name", "ExecuteQuery"}
            }
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

TEST(OperationMetricsAliasesTest, TableOperationsUseOtelStandardMetrics) {
    auto registry = std::make_shared<TFakeMetricRegistry>();
    TStatCollector::TClientOperationStatCollector collector;
    collector.SetExternalRegistry(registry);

    NObservability::TOperationMetrics metrics(&collector, "ExecuteDataQuery", TLog());
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
