#pragma once

#include <memory>

#include "envoy/stats/store.h"
#include "envoy/tracing/http_tracer.h"

#include "common/common/logger.h"
#include "common/singleton/const_singleton.h"

#include "opentracing/ext/tags.h"
#include "opentracing/tracer.h"

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Common {
namespace Ot {

#define OPENTRACING_TRACER_STATS(COUNTER)                                                          \
  COUNTER(span_context_extraction_error)                                                           \
  COUNTER(span_context_injection_error)

struct OpenTracingTracerStats {
  OPENTRACING_TRACER_STATS(GENERATE_COUNTER_STRUCT)
};

class OpenTracingDriver;

class OpenTracingSpan : public Tracing::Span, Logger::Loggable<Logger::Id::tracing> {
public:
  OpenTracingSpan(OpenTracingDriver& driver, std::unique_ptr<opentracing::Span>&& span);

  // Tracing::Span
  void finishSpan() override;
  void setOperation(const std::string& operation) override;
  void setTag(const std::string& name, const std::string& value) override;
  void log(SystemTime timestamp, const std::string& event) override;
  void injectContext(Http::HeaderMap& request_headers) override;
  Tracing::SpanPtr spawnChild(const Tracing::Config& config, const std::string& name,
                              SystemTime start_time) override;
  void setSampled(bool) override;

private:
  OpenTracingDriver& driver_;
  opentracing::FinishSpanOptions finish_options_;
  std::unique_ptr<opentracing::Span> span_;
};

/**
 * This driver can be used by tracing libraries implementing the OpenTracing API (see
 * https://github.com/opentracing/opentracing-cpp) to hook into Envoy's tracing functionality with a
 * minimal amount of effort. Libraries need only provide an opentracing::Tracer implementation; the
 * rest of span creation is taken care of by this class.
 */
class OpenTracingDriver : public Tracing::Driver, protected Logger::Loggable<Logger::Id::tracing> {
public:
  explicit OpenTracingDriver(Stats::Store& stats);

  bool dropLogs() const { return drop_logs_; }

  // Tracer::TracingDriver
  Tracing::SpanPtr startSpan(const Tracing::Config& config, Http::HeaderMap& request_headers,
                             const std::string& operation_name, SystemTime start_time,
                             const Tracing::Decision tracing_decision) override;

  void setDropLogs(bool drop_logs) override { drop_logs_ = drop_logs; }

  virtual opentracing::Tracer& tracer() PURE;

  enum class PropagationMode { SingleHeader, TracerNative };

  /**
   * Controls how span context is propagated in HTTP headers. PropagationMode::SingleHeader will
   * propagate span context as a single header within the inline header HeaderMap::OtSpanContext;
   * otherwise, span context will be propagated using the native format of the tracing library.
   */
  virtual PropagationMode propagationMode() const { return PropagationMode::SingleHeader; }

  OpenTracingTracerStats& tracerStats() { return tracer_stats_; }

private:
  OpenTracingTracerStats tracer_stats_;
  bool drop_logs_{true};
};
} // namespace Ot
} // namespace Common
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
