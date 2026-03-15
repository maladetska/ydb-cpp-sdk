#pragma once

#include <library/cpp/logger/log.h>

#include <src/client/impl/internal/internal_header.h>
#include <src/client/impl/internal/common/balancing_policies.h>
#include <src/client/impl/internal/common/types.h>
#include <ydb-cpp-sdk/client/common_client/ssl_credentials.h>
#include <ydb-cpp-sdk/client/types/credentials/credentials.h>
#include <ydb-cpp-sdk/client/types/executor/executor.h>

namespace NYdb::inline V3 {

namespace NMetrics {
    class IMetricRegistry;
    class ITraceProvider;
} // namespace NMetrics

class IConnectionsParams {
public:
    virtual ~IConnectionsParams() = default;
    virtual std::string GetEndpoint() const = 0;
    virtual size_t GetNetworkThreadsNum() const = 0;
    virtual size_t GetClientThreadsNum() const = 0;
    virtual size_t GetMaxQueuedResponses() const = 0;
    virtual TSslCredentials GetSslCredentials() const = 0;
    virtual bool GetUsePerChannelTcpConnection() const = 0;
    virtual std::string GetDatabase() const = 0;
    virtual std::shared_ptr<ICredentialsProviderFactory> GetCredentialsProviderFactory() const = 0;
    virtual EDiscoveryMode GetDiscoveryMode() const = 0;
    virtual size_t GetMaxQueuedRequests() const = 0;
    virtual NYdbGrpc::TTcpKeepAliveSettings GetTcpKeepAliveSettings() const = 0;
    virtual bool GetTcpNoDelay() const = 0;
    virtual bool GetDrinOnDtors() const = 0;
    virtual TBalancingPolicy::TImpl GetBalancingSettings() const = 0;
    virtual TDuration GetGRpcKeepAliveTimeout() const = 0;
    virtual bool GetGRpcKeepAlivePermitWithoutCalls() const = 0;
    virtual std::string GetGRpcLoadBalancingPolicy() const = 0;
    virtual TDuration GetSocketIdleTimeout() const = 0;
    virtual const TLog& GetLog() const = 0;
    virtual uint64_t GetMemoryQuota() const = 0;
    virtual uint64_t GetMaxInboundMessageSize() const = 0;
    virtual uint64_t GetMaxOutboundMessageSize() const = 0;
    virtual uint64_t GetMaxMessageSize() const = 0;
    virtual std::shared_ptr<IExecutor> GetExecutor() const = 0;
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    virtual std::shared_ptr<NMetrics::IMetricRegistry> GetExternalMetricRegistry() const = 0;
    virtual std::shared_ptr<NMetrics::ITraceProvider> GetTraceProvider() const = 0;
=======
    virtual std::shared_ptr<NMetrics::IMetricRegistry> GetMetricExporter() const = 0;
    virtual std::shared_ptr<NMetrics::ITraceProvider> GetTraceExporter() const = 0;
>>>>>>> 1b2bf4fa5 (fixes)
=======
    virtual std::shared_ptr<NMetrics::IMetricRegistry> GetExternalMetricRegistry() const = 0;
    virtual std::shared_ptr<NMetrics::ITraceProvider> GetTraceProvider() const = 0;
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
    virtual std::shared_ptr<NMetrics::IMetricRegistry> GetMetricExporter() const = 0;
    virtual std::shared_ptr<NMetrics::ITraceProvider> GetTraceExporter() const = 0;
>>>>>>> a979e6bda (fixes)
=======
    virtual std::shared_ptr<NMetrics::IMetricRegistry> GetExternalMetricRegistry() const = 0;
    virtual std::shared_ptr<NMetrics::ITraceProvider> GetTraceProvider() const = 0;
>>>>>>> dcae6d69e (fixes and add metric tests)
};

} // namespace NYdb
