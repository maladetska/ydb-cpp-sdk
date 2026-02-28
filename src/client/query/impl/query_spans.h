#pragma once

#include <ydb-cpp-sdk/client/trace/trace.h>
#include <ydb-cpp-sdk/client/types/status_codes.h>
#include <ydb-cpp-sdk/client/types/status/status.h>

#include <map>
#include <memory>
#include <string>

namespace NYdb::inline V3::NQuery {

class TQuerySpan {
public:
    TQuerySpan(std::shared_ptr<NMetrics::ITracer> tracer, const std::string& operationName, const std::string& endpoint);
    ~TQuerySpan() noexcept;

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    void SetPeerEndpoint(const std::string& endpoint) noexcept;
    void AddEvent(const std::string& name, const std::map<std::string, std::string>& attributes = {}) noexcept;
    std::unique_ptr<NMetrics::IScope> Activate() noexcept;

=======
>>>>>>> 1b2bf4fa5 (fixes)
=======
    void SetPeerEndpoint(const std::string& endpoint) noexcept;
    void SetQueryText(const std::string& query) noexcept;
    void AddEvent(const std::string& name, const std::map<std::string, std::string>& attributes = {}) noexcept;

>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> a979e6bda (fixes)
    void End(EStatus status) noexcept;

private:
    std::shared_ptr<NMetrics::ISpan> Span_;
};

} // namespace NYdb::NQuery
