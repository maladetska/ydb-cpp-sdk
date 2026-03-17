#include <ydb-cpp-sdk/client/driver/driver.h>
#include <ydb-cpp-sdk/client/query/client.h>
#include <ydb-cpp-sdk/client/query/tx.h>
#include <ydb-cpp-sdk/client/retry/retry.h>

#include <ydb-cpp-sdk/open_telemetry/trace.h>
#include <ydb-cpp-sdk/open_telemetry/metrics.h>

#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/metrics/view/view_registry.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h>
#include <opentelemetry/sdk/resource/resource.h>

#include <library/cpp/getopt/last_getopt.h>

#include <chrono>
#include <iostream>
#include <thread>

namespace nostd = opentelemetry::nostd;
namespace sdktrace = opentelemetry::sdk::trace;
namespace sdkmetrics = opentelemetry::sdk::metrics;
namespace otlp = opentelemetry::exporter::otlp;
namespace resource = opentelemetry::sdk::resource;

using namespace NYdb;
using namespace NYdb::NQuery;
using namespace NYdb::NStatusHelpers;

struct TConfig {
    std::string Endpoint = "grpc://localhost:2136";
    std::string Database = "/local";
    std::string OtlpEndpoint = "http://localhost:4328";
    int Iterations = 20;
};

nostd::shared_ptr<opentelemetry::trace::TracerProvider> InitTracing(const TConfig& cfg) {
    otlp::OtlpHttpExporterOptions opts;
    opts.url = cfg.OtlpEndpoint + "/v1/traces";

    auto exporter = otlp::OtlpHttpExporterFactory::Create(opts);
    auto processor = sdktrace::SimpleSpanProcessorFactory::Create(std::move(exporter));

    auto res = resource::Resource::Create({
        {"service.name", "ydb-cpp-sdk-demo"},
        {"service.version", "1.0.0"},
    });

    std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
        std::make_shared<sdktrace::TracerProvider>(std::move(processor), res);
    return nostd::shared_ptr<opentelemetry::trace::TracerProvider>(provider);
}

nostd::shared_ptr<opentelemetry::metrics::MeterProvider> InitMetrics(const TConfig& cfg) {
    otlp::OtlpHttpMetricExporterOptions opts;
    opts.url = cfg.OtlpEndpoint + "/v1/metrics";

    auto exporter = otlp::OtlpHttpMetricExporterFactory::Create(opts);

    sdkmetrics::PeriodicExportingMetricReaderOptions readerOpts;
    readerOpts.export_interval_millis = std::chrono::milliseconds(5000);
    readerOpts.export_timeout_millis = std::chrono::milliseconds(3000);

    auto reader = sdkmetrics::PeriodicExportingMetricReaderFactory::Create(std::move(exporter), readerOpts);

    auto res = resource::Resource::Create({
        {"service.name", "ydb-cpp-sdk-demo"},
        {"service.version", "1.0.0"},
    });

    auto rawProvider = std::make_shared<sdkmetrics::MeterProvider>(
        std::unique_ptr<sdkmetrics::ViewRegistry>(new sdkmetrics::ViewRegistry()), res);
    rawProvider->AddMetricReader(std::move(reader));

    std::shared_ptr<opentelemetry::metrics::MeterProvider> provider = rawProvider;
    return nostd::shared_ptr<opentelemetry::metrics::MeterProvider>(provider);
}

void RunWorkload(TQueryClient& client, int iterations) {
    std::cout << "=== Creating table ===" << std::endl;

    ThrowOnError(client.RetryQuerySync([](TSession session) {
        return session.ExecuteQuery(R"(
            CREATE TABLE IF NOT EXISTS otel_demo (
                id Uint64,
                value Utf8,
                PRIMARY KEY (id)
            )
        )", TTxControl::NoTx()).GetValueSync();
    }));

    for (int i = 0; i < iterations; ++i) {
        std::cout << "--- Iteration " << (i + 1) << "/" << iterations << " ---" << std::endl;

        ThrowOnError(client.RetryQuerySync([i](TSession session) {
            auto params = TParamsBuilder()
                .AddParam("$id").Uint64(i).Build()
                .AddParam("$val").Utf8("item_" + std::to_string(i)).Build()
                .Build();

            return session.ExecuteQuery(R"(
                DECLARE $id AS Uint64;
                DECLARE $val AS Utf8;
                UPSERT INTO otel_demo (id, value) VALUES ($id, $val)
            )", TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx(),
                params).GetValueSync();
        }));

        ThrowOnError(client.RetryQuerySync([i](TSession session) {
            auto params = TParamsBuilder()
                .AddParam("$id").Uint64(i).Build()
                .Build();

            return session.ExecuteQuery(R"(
                DECLARE $id AS Uint64;
                SELECT id, value FROM otel_demo WHERE id = $id
            )", TTxControl::BeginTx(TTxSettings::SerializableRW()).CommitTx(),
                params).GetValueSync();
        }));

        ThrowOnError(client.RetryQuerySync([](TQueryClient client) -> TStatus {
            auto session = client.GetSession().GetValueSync().GetSession();
            auto beginResult = session.BeginTransaction(TTxSettings::SerializableRW()).GetValueSync();
            if (!beginResult.IsSuccess()) {
                return beginResult;
            }
            auto tx = beginResult.GetTransaction();

            auto result = session.ExecuteQuery(R"(
                SELECT COUNT(*) AS cnt FROM otel_demo
            )", TTxControl::Tx(tx)).GetValueSync();

            if (!result.IsSuccess()) {
                return result;
            }

            return tx.Commit().GetValueSync();
        }));

        if (i % 5 == 4) {
            auto rollbackResult = client.RetryQuerySync([](TQueryClient client) -> TStatus {
                auto session = client.GetSession().GetValueSync().GetSession();
                auto beginResult = session.BeginTransaction(TTxSettings::SerializableRW()).GetValueSync();
                if (!beginResult.IsSuccess()) {
                    return beginResult;
                }
                auto tx = beginResult.GetTransaction();
                return tx.Rollback().GetValueSync();
            });
            if (!rollbackResult.IsSuccess()) {
                std::cerr << "  Rollback status: " << static_cast<int>(rollbackResult.GetStatus()) << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "=== Dropping table ===" << std::endl;

    ThrowOnError(client.RetryQuerySync([](TSession session) {
        return session.ExecuteQuery(
            "DROP TABLE otel_demo", TTxControl::NoTx()).GetValueSync();
    }));
}

int main(int argc, char** argv) {
    TConfig cfg;

    NLastGetopt::TOpts opts;
    opts.AddLongOption('e', "endpoint", "YDB endpoint")
        .DefaultValue(cfg.Endpoint).StoreResult(&cfg.Endpoint);
    opts.AddLongOption('d', "database", "YDB database")
        .DefaultValue(cfg.Database).StoreResult(&cfg.Database);
    opts.AddLongOption("otlp", "OTLP HTTP endpoint")
        .DefaultValue(cfg.OtlpEndpoint).StoreResult(&cfg.OtlpEndpoint);
    opts.AddLongOption('n', "iterations", "Number of iterations")
        .DefaultValue(std::to_string(cfg.Iterations)).StoreResult(&cfg.Iterations);

    NLastGetopt::TOptsParseResult parsedOpts(&opts, argc, argv);

    std::cout << "Initializing OpenTelemetry..." << std::endl;
    std::cout << "  OTLP endpoint: " << cfg.OtlpEndpoint << std::endl;

    auto tracerProvider = InitTracing(cfg);
    auto meterProvider = InitMetrics(cfg);

    auto ydbTraceProvider = NMetrics::CreateOtelTraceProvider(tracerProvider);
    auto ydbMetricRegistry = NMetrics::CreateOtelMetricRegistry(meterProvider);

    std::cout << "Connecting to YDB at " << cfg.Endpoint << cfg.Database << std::endl;

    auto driverConfig = TDriverConfig()
        .SetEndpoint(cfg.Endpoint)
        .SetDatabase(cfg.Database)
        .SetDiscoveryMode(EDiscoveryMode::Off)
        .SetTraceProvider(ydbTraceProvider)
        .SetMetricRegistry(ydbMetricRegistry);

    TDriver driver(driverConfig);
    TQueryClient client(driver);

    try {
        RunWorkload(client, cfg.Iterations);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "Flushing telemetry..." << std::endl;

    driver.Stop(true);

    if (auto* sdkTracerProvider = dynamic_cast<sdktrace::TracerProvider*>(tracerProvider.get())) {
        sdkTracerProvider->ForceFlush();
    }
    if (auto* sdkMeterProvider = dynamic_cast<sdkmetrics::MeterProvider*>(meterProvider.get())) {
        sdkMeterProvider->ForceFlush();
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "Done. Open Grafana at http://localhost:3000" << std::endl;
    std::cout << "  Dashboard: YDB QueryService" << std::endl;
    std::cout << "  Also: Jaeger UI at http://localhost:16686" << std::endl;

    return 0;
}
