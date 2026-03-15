#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/query/client.h>
#include <tests/common/fake_metric_registry.h>
<<<<<<< HEAD
<<<<<<< HEAD
#include <util/string/cast.h>
=======
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

#include <library/cpp/testing/gtest/gtest.h>

using namespace NYdb;
using namespace NYdb::NQuery;
using namespace NYdb::NTests;

namespace {

struct TRunArgs {
    TDriver Driver;
    std::shared_ptr<TFakeMetricRegistry> Registry;
};

TRunArgs MakeRunArgs() {
    std::string endpoint = std::getenv("YDB_ENDPOINT");
    std::string database = std::getenv("YDB_DATABASE");

    auto registry = std::make_shared<TFakeMetricRegistry>();

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(std::getenv("YDB_TOKEN") ? std::getenv("YDB_TOKEN") : "")
        .SetMetricRegistry(registry);

    TDriver driver(driverConfig);
    return {driver, registry};
}

std::shared_ptr<TFakeCounter> GetCounter(
    const std::shared_ptr<TFakeMetricRegistry>& registry,
    const std::string& name,
    const std::string& operation)
{
<<<<<<< HEAD
<<<<<<< HEAD
    return registry->GetCounter(name, {
        {"db.system.name", "other_sql"},
        {"db.operation.name", operation},
    });
}

std::shared_ptr<TFakeHistogram> GetDuration(
    const std::shared_ptr<TFakeMetricRegistry>& registry,
    const std::string& operation,
    EStatus status)
{
    NMetrics::TLabels labels = {
        {"db.system.name", "other_sql"},
        {"db.operation.name", operation},
        {"db.response.status_code", ToString(status)},
    };
    if (status != EStatus::SUCCESS) {
        labels["error.type"] = ToString(status);
    }
    return registry->GetHistogram("db.client.operation.duration", labels);
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    return registry->GetCounter(name, {{"operation", operation}});
}

std::shared_ptr<TFakeHistogram> GetHistogram(
    const std::shared_ptr<TFakeMetricRegistry>& registry,
    const std::string& name,
    const std::string& operation)
{
    return registry->GetHistogram(name, {{"operation", operation}});
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
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

<<<<<<< HEAD
<<<<<<< HEAD
    auto requests = GetCounter(registry, "db.client.operation.requests", "ExecuteQuery");
    ASSERT_NE(requests, nullptr) << "ExecuteQuery request counter not created";
    EXPECT_GE(requests->Get(), 1);

    auto errors = GetCounter(registry, "db.client.operation.errors", "ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto duration = GetDuration(registry, "ExecuteQuery", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr) << "ExecuteQuery duration histogram not created";
    EXPECT_GE(duration->Count(), 1u);
    for (double v : duration->GetValues()) {
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    auto requests = GetCounter(registry, "ydb.query.requests", "ExecuteQuery");
    ASSERT_NE(requests, nullptr) << "ExecuteQuery request counter not created";
    EXPECT_GE(requests->Get(), 1);

    auto errors = GetCounter(registry, "ydb.query.errors", "ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto latency = GetHistogram(registry, "ydb.query.latency_ms", "ExecuteQuery");
    ASSERT_NE(latency, nullptr) << "ExecuteQuery latency histogram not created";
    EXPECT_GE(latency->Count(), 1u);
    for (double v : latency->GetValues()) {
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
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

<<<<<<< HEAD
<<<<<<< HEAD
    auto requests = GetCounter(registry, "db.client.operation.requests", "ExecuteQuery");
    ASSERT_NE(requests, nullptr);
    EXPECT_GE(requests->Get(), 1);

    auto errors = GetCounter(registry, "db.client.operation.errors", "ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_GE(errors->Get(), 1);

    auto duration = GetDuration(registry, "ExecuteQuery", result.GetStatus());
    ASSERT_NE(duration, nullptr);
    EXPECT_GE(duration->Count(), 1u);
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    auto requests = GetCounter(registry, "ydb.query.requests", "ExecuteQuery");
    ASSERT_NE(requests, nullptr);
    EXPECT_GE(requests->Get(), 1);

    auto errors = GetCounter(registry, "ydb.query.errors", "ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_GE(errors->Get(), 1);

    auto latency = GetHistogram(registry, "ydb.query.latency_ms", "ExecuteQuery");
    ASSERT_NE(latency, nullptr);
    EXPECT_GE(latency->Count(), 1u);
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, CreateSessionRecordsMetrics) {
    auto [driver, registry] = MakeRunArgs();
    TQueryClient client(driver);

    auto session = client.GetSession().ExtractValueSync();
    ASSERT_TRUE(session.IsSuccess()) << session.GetIssues().ToString();

<<<<<<< HEAD
<<<<<<< HEAD
    auto requests = GetCounter(registry, "db.client.operation.requests", "GetSession");
    ASSERT_NE(requests, nullptr) << "CreateSession request counter not created";
    EXPECT_GE(requests->Get(), 1);

    auto duration = GetDuration(registry, "GetSession", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr) << "CreateSession duration histogram not created";
    EXPECT_GE(duration->Count(), 1u);
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    auto requests = GetCounter(registry, "ydb.query.requests", "CreateSession");
    ASSERT_NE(requests, nullptr) << "CreateSession request counter not created";
    EXPECT_GE(requests->Get(), 1);

    auto latency = GetHistogram(registry, "ydb.query.latency_ms", "CreateSession");
    ASSERT_NE(latency, nullptr) << "CreateSession latency histogram not created";
    EXPECT_GE(latency->Count(), 1u);
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

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

<<<<<<< HEAD
<<<<<<< HEAD
        auto commitRequests = GetCounter(registry, "db.client.operation.requests", "Commit");
        ASSERT_NE(commitRequests, nullptr) << "Commit request counter not created";
        EXPECT_GE(commitRequests->Get(), 1);

        auto commitDuration = GetDuration(registry, "Commit", EStatus::SUCCESS);
        ASSERT_NE(commitDuration, nullptr);
        EXPECT_GE(commitDuration->Count(), 1u);
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
        auto commitRequests = GetCounter(registry, "ydb.query.requests", "Commit");
        ASSERT_NE(commitRequests, nullptr) << "Commit request counter not created";
        EXPECT_GE(commitRequests->Get(), 1);

        auto commitLatency = GetHistogram(registry, "ydb.query.latency_ms", "Commit");
        ASSERT_NE(commitLatency, nullptr);
        EXPECT_GE(commitLatency->Count(), 1u);
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
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

<<<<<<< HEAD
<<<<<<< HEAD
    auto rollbackRequests = GetCounter(registry, "db.client.operation.requests", "Rollback");
    ASSERT_NE(rollbackRequests, nullptr) << "Rollback request counter not created";
    EXPECT_GE(rollbackRequests->Get(), 1);

    auto rollbackErrors = GetCounter(registry, "db.client.operation.errors", "Rollback");
    ASSERT_NE(rollbackErrors, nullptr);
    EXPECT_EQ(rollbackErrors->Get(), 0);

    auto rollbackDuration = GetDuration(registry, "Rollback", EStatus::SUCCESS);
    ASSERT_NE(rollbackDuration, nullptr);
    EXPECT_GE(rollbackDuration->Count(), 1u);
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    auto rollbackRequests = GetCounter(registry, "ydb.query.requests", "Rollback");
    ASSERT_NE(rollbackRequests, nullptr) << "Rollback request counter not created";
    EXPECT_GE(rollbackRequests->Get(), 1);

    auto rollbackErrors = GetCounter(registry, "ydb.query.errors", "Rollback");
    ASSERT_NE(rollbackErrors, nullptr);
    EXPECT_EQ(rollbackErrors->Get(), 0);

    auto rollbackLatency = GetHistogram(registry, "ydb.query.latency_ms", "Rollback");
    ASSERT_NE(rollbackLatency, nullptr);
    EXPECT_GE(rollbackLatency->Count(), 1u);
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

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

<<<<<<< HEAD
<<<<<<< HEAD
    auto requests = GetCounter(registry, "db.client.operation.requests", "ExecuteQuery");
    ASSERT_NE(requests, nullptr);
    EXPECT_EQ(requests->Get(), numQueries);

    auto errors = GetCounter(registry, "db.client.operation.errors", "ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto duration = GetDuration(registry, "ExecuteQuery", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr);
    EXPECT_EQ(duration->Count(), static_cast<size_t>(numQueries));
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    auto requests = GetCounter(registry, "ydb.query.requests", "ExecuteQuery");
    ASSERT_NE(requests, nullptr);
    EXPECT_EQ(requests->Get(), numQueries);

    auto errors = GetCounter(registry, "ydb.query.errors", "ExecuteQuery");
    ASSERT_NE(errors, nullptr);
    EXPECT_EQ(errors->Get(), 0);

    auto latency = GetHistogram(registry, "ydb.query.latency_ms", "ExecuteQuery");
    ASSERT_NE(latency, nullptr);
    EXPECT_EQ(latency->Count(), static_cast<size_t>(numQueries));
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)

    driver.Stop(true);
}

TEST(QueryMetricsIntegration, NoRegistryDoesNotBreakOperations) {
    std::string endpoint = std::getenv("YDB_ENDPOINT");
    std::string database = std::getenv("YDB_DATABASE");

    auto driverConfig = TDriverConfig()
        .SetEndpoint(endpoint)
        .SetDatabase(database)
        .SetAuthToken(std::getenv("YDB_TOKEN") ? std::getenv("YDB_TOKEN") : "");

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

<<<<<<< HEAD
<<<<<<< HEAD
TEST(QueryMetricsIntegration, DurationValuesAreRealistic) {
=======
TEST(QueryMetricsIntegration, LatencyValuesAreRealistic) {
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
TEST(QueryMetricsIntegration, LatencyValuesAreRealistic) {
>>>>>>> dcae6d69e (fixes and add metric tests)
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

<<<<<<< HEAD
<<<<<<< HEAD
    auto duration = GetDuration(registry, "ExecuteQuery", EStatus::SUCCESS);
    ASSERT_NE(duration, nullptr);
    ASSERT_GE(duration->Count(), 1u);

    for (double v : duration->GetValues()) {
        EXPECT_GE(v, 0.0) << "Duration must be non-negative";
        EXPECT_LT(v, 30.0) << "Duration > 30s is unrealistic for SELECT 1";
=======
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    auto latency = GetHistogram(registry, "ydb.query.latency_ms", "ExecuteQuery");
    ASSERT_NE(latency, nullptr);
    ASSERT_GE(latency->Count(), 1u);

    for (double v : latency->GetValues()) {
        EXPECT_GE(v, 0.0) << "Latency must be non-negative";
        EXPECT_LT(v, 30000.0) << "Latency > 30s is unrealistic for SELECT 1";
<<<<<<< HEAD
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> dcae6d69e (fixes and add metric tests)
    }

    driver.Stop(true);
}
