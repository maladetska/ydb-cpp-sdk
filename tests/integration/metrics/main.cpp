#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/query/client.h>
#include <tests/common/fake_metric_registry.h>
#include <util/string/cast.h>

#include <cstdlib>

#include <library/cpp/testing/gtest/gtest.h>

using namespace NYdb;
using namespace NYdb::NQuery;
using namespace NYdb::NTests;

namespace {

std::string GetEnvOrEmpty(const char* name) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : std::string();
}

struct TRunArgs {
    TDriver Driver;
    std::shared_ptr<TFakeMetricRegistry> Registry;
};

TRunArgs MakeRunArgs() {
    std::string endpoint = GetEnvOrEmpty("YDB_ENDPOINT");
    std::string database = GetEnvOrEmpty("YDB_DATABASE");

    auto registry = std::make_shared<TFakeMetricRegistry>();

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(GetEnvOrEmpty("YDB_TOKEN"))
        .SetMetricRegistry(registry);

    TDriver driver(driverConfig);
    return {driver, registry};
}

std::shared_ptr<TFakeCounter> GetFailedCounter(
    const std::shared_ptr<TFakeMetricRegistry>& registry,
    const std::string& operation,
    EStatus status)
{
    const std::string statusName = ToString(status);
    return registry->GetCounter("db.client.operation.failed", {
        {"db.system.name", "ydb"},
        {"db.namespace", GetEnvOrEmpty("YDB_DATABASE")},
        {"db.operation.name", operation},
        {"ydb.client.api", "Query"},
        {"db.response.status_code", statusName},
        {"error.type", statusName},
    });
}

// Label set must match NSdkStats::TClientOperationStatCollector::RecordLatency (stats.h) and
// unit tests in tests/unit/client/observability/metrics_ut.cpp: success has no
// db.response.status_code; errors add status and error.type.
std::shared_ptr<TFakeHistogram> GetDuration(
    const std::shared_ptr<TFakeMetricRegistry>& registry,
    const std::string& operation,
    EStatus status)
{
    NMetrics::TLabels labels = {
        {"db.system.name", "ydb"},
        {"db.namespace", GetEnvOrEmpty("YDB_DATABASE")},
        {"db.operation.name", operation},
        {"ydb.client.api", "Query"},
    };
    if (status != EStatus::SUCCESS) {
        const std::string statusName = ToString(status);
        labels["db.response.status_code"] = statusName;
        labels["error.type"] = statusName;
    }
    return registry->GetHistogram("db.client.operation.duration", labels);
}

} // namespace

TEST(QueryMetricsIntegration, ExecuteQuerySuccessRecordsMetrics) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto session = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(session.IsSuccess()) << session.GetIssues().ToString();

    auto result = session.GetSession().ExecuteQuery(
        "SELECT 1;",
        TTxControl::BeginTx().CommitTx()
    ).ExtractValueSync();
    ASSERT_EQ(result.GetStatus(), EStatus::SUCCESS) << result.GetIssues().ToString();

    auto duration = GetDuration(registry, "ydb.ExecuteQuery", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr) << "ExecuteQuery duration histogram not created";
    EXPECT_GE(duration->Count(), 1u);
    for (double v : duration->GetValues()) {
        EXPECT_GE(v, 0.0);
    }

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, ExecuteQueryErrorRecordsErrorMetric) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto session = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(session.IsSuccess()) << session.GetIssues().ToString();

    auto result = session.GetSession().ExecuteQuery(
        "INVALID SQL QUERY !!!",
        TTxControl::BeginTx().CommitTx()
    ).ExtractValueSync();
    EXPECT_NE(result.GetStatus(), EStatus::SUCCESS);

    auto failed = GetFailedCounter(registry, "ydb.ExecuteQuery", result.GetStatus());
    ASSERT_NE(failed, nullptr);
    EXPECT_GE(failed->Get(), 1);

    auto duration = GetDuration(registry, "ydb.ExecuteQuery", result.GetStatus());
    ASSERT_NE(duration, nullptr);
    EXPECT_GE(duration->Count(), 1u);

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, CreateSessionRecordsMetrics) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto session = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(session.IsSuccess()) << session.GetIssues().ToString();

    // Query client uses observation name "CreateSession" -> ydb.CreateSession in metrics.
    auto duration = GetDuration(registry, "ydb.CreateSession", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr) << "CreateSession duration histogram not created";
    EXPECT_GE(duration->Count(), 1u);

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, CommitTransactionRecordsMetrics) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto sessionResult = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(sessionResult.IsSuccess()) << sessionResult.GetIssues().ToString();
    auto session = sessionResult.GetSession();

    auto beginResult = session.BeginTransaction(TTxSettings::SerializableRW()).ExtractValueSync();
    ASSERT_TRUE(beginResult.IsSuccess()) << beginResult.GetIssues().ToString();
    auto tx = beginResult.GetTransaction();

    auto execResult = tx.GetSession().ExecuteQuery(
        "SELECT 1;",
        TTxControl::Tx(tx)
    ).ExtractValueSync();
    ASSERT_EQ(execResult.GetStatus(), EStatus::SUCCESS) << execResult.GetIssues().ToString();

    if (execResult.GetTransaction()) {
        auto commitResult = execResult.GetTransaction()->Commit().ExtractValueSync();
        ASSERT_TRUE(commitResult.IsSuccess()) << commitResult.GetIssues().ToString();

        auto commitDuration = GetDuration(registry, "ydb.Commit", EStatus::SUCCESS);
        ASSERT_NE(commitDuration, nullptr);
        EXPECT_GE(commitDuration->Count(), 1u);
    }

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, RollbackTransactionRecordsMetrics) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto sessionResult = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(sessionResult.IsSuccess()) << sessionResult.GetIssues().ToString();
    auto session = sessionResult.GetSession();

    auto beginResult = session.BeginTransaction(TTxSettings::SerializableRW()).ExtractValueSync();
    ASSERT_TRUE(beginResult.IsSuccess()) << beginResult.GetIssues().ToString();
    auto tx = beginResult.GetTransaction();

    auto rollbackResult = tx.Rollback().ExtractValueSync();
    ASSERT_TRUE(rollbackResult.IsSuccess()) << rollbackResult.GetIssues().ToString();

    auto rollbackDuration = GetDuration(registry, "ydb.Rollback", EStatus::SUCCESS);
    ASSERT_NE(rollbackDuration, nullptr);
    EXPECT_GE(rollbackDuration->Count(), 1u);

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, MultipleQueriesAccumulateMetrics) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto sessionResult = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(sessionResult.IsSuccess()) << sessionResult.GetIssues().ToString();
    auto session = sessionResult.GetSession();

    const int numQueries = 5;
    for (int i = 0; i < numQueries; ++i) {
        auto result = session.ExecuteQuery(
            "SELECT 1;",
            TTxControl::BeginTx().CommitTx()
        ).ExtractValueSync();
        ASSERT_EQ(result.GetStatus(), EStatus::SUCCESS) << result.GetIssues().ToString();
    }

    auto duration = GetDuration(registry, "ydb.ExecuteQuery", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr);
    EXPECT_EQ(duration->Count(), static_cast<size_t>(numQueries));

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, NoRegistryDoesNotBreakOperations) {
    std::string endpoint = GetEnvOrEmpty("YDB_ENDPOINT");
    std::string database = GetEnvOrEmpty("YDB_DATABASE");

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(GetEnvOrEmpty("YDB_TOKEN"));

    TDriver driver(driverConfig);
    TQueryClient client(driver);

    auto session = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(session.IsSuccess()) << session.GetIssues().ToString();

    auto result = session.GetSession().ExecuteQuery(
        "SELECT 1;",
        TTxControl::BeginTx().CommitTx()
    ).ExtractValueSync();
    EXPECT_EQ(result.GetStatus(), EStatus::SUCCESS) << result.GetIssues().ToString();

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, DurationValuesAreRealistic) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto sessionResult = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(sessionResult.IsSuccess()) << sessionResult.GetIssues().ToString();
    auto session = sessionResult.GetSession();

    auto result = session.ExecuteQuery(
        "SELECT 1;",
        TTxControl::BeginTx().CommitTx()
    ).ExtractValueSync();
    ASSERT_EQ(result.GetStatus(), EStatus::SUCCESS) << result.GetIssues().ToString();

    auto duration = GetDuration(registry, "ydb.ExecuteQuery", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr);
    ASSERT_GE(duration->Count(), 1u);

    for (double v : duration->GetValues()) {
        EXPECT_GE(v, 0.0) << "Duration must be non-negative";
        EXPECT_LT(v, 30.0) << "Duration > 30s is unrealistic for SELECT 1";
    }

    driver.Stop(true);
}
