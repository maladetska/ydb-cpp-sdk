#pragma once

<<<<<<< HEAD
<<<<<<< HEAD
#include <cstdint>
=======
>>>>>>> 1b2bf4fa5 (fixes)
=======
>>>>>>> a979e6bda (fixes)
#include <map>

namespace NYdb::inline V3::NMetrics {

using TLabels = std::map<std::string, std::string>;

class ICounter {
public:
    virtual ~ICounter() = default;
    virtual void Inc() = 0;
};

class IGauge {
public:
    virtual ~IGauge() = default;
    virtual void Add(double delta) = 0;
    virtual void Set(double value) = 0;
};

class IHistogram {
public:
    virtual ~IHistogram() = default;
    virtual void Record(double value) = 0;
};

class IMetricRegistry {
public:
    virtual ~IMetricRegistry() = default;

<<<<<<< HEAD
    virtual std::shared_ptr<ICounter> Counter(
        const std::string& name,
        const TLabels& labels = {},
        const std::string& description = {},
        const std::string& unit = {}
    ) = 0;
    virtual std::shared_ptr<IGauge> Gauge(
        const std::string& name,
        const TLabels& labels = {},
        const std::string& description = {},
        const std::string& unit = {}
    ) = 0;
    virtual std::shared_ptr<IHistogram> Histogram(
        const std::string& name,
        const std::vector<double>& buckets,
        const TLabels& labels = {},
        const std::string& description = {},
        const std::string& unit = {}
    ) = 0;
=======
    virtual std::shared_ptr<ICounter> Counter(const std::string& name, const TLabels& labels = {}) = 0;
    virtual std::shared_ptr<IGauge> Gauge(const std::string& name, const TLabels& labels = {}) = 0;
    virtual std::shared_ptr<IHistogram> Histogram(const std::string& name, const std::vector<double>& buckets, const TLabels& labels = {}) = 0;
};

<<<<<<< HEAD
enum class ESpanKind {
    INTERNAL,
    SERVER,
    CLIENT,
    PRODUCER,
    CONSUMER
};

class ISpan {
public:
    virtual ~ISpan() = default;
    virtual void End() = 0;
    virtual void SetAttribute(const std::string& key, const std::string& value) = 0;
    virtual void SetAttribute(const std::string& key, int64_t value) = 0;
};

class ITracer {
public:
    virtual ~ITracer() = default;
    virtual std::shared_ptr<ISpan> StartSpan(const std::string& name, ESpanKind kind = ESpanKind::INTERNAL) = 0;
};

class ITraceProvider {
public:
    virtual ~ITraceProvider() = default;
    virtual std::shared_ptr<ITracer> GetTracer(const std::string& name) = 0;
>>>>>>> 1b2bf4fa5 (fixes)
};

<<<<<<< HEAD
=======
>>>>>>> 1ca4253b5 (fixes and add metric tests)
=======
>>>>>>> a979e6bda (fixes)
} // namespace NYdb::NMetrics
