#pragma once

#include "envoy/tracing/http_tracer.h"

#include "opentracing/span.h"

namespace Envoy {
namespace Tracing {

class OpenTracingSpan : public Span {
public:
  explicit OpenTracingSpan(std::unique_ptr<opentracing::Span>&& span);

  // Tracing::Span
  void finishSpan(SpanFinalizer& finalizer) override;
  void setTag(const std::string& name, const std::string& value) override;
  void injectContext(Http::HeaderMap& request_headers) override;
  SpanPtr spawnChild(const std::string& name, SystemTime start_time) override;

private:
  std::unique_ptr<opentracing::Span> span_;
};

class OpenTracingDriver : public Driver {
public:
  // Tracer::TracingDriver
  SpanPtr startSpan(Http::HeaderMap& request_headers, const std::string& operation_name,
                    SystemTime start_time) override;
};

} // namespace Tracing
} // namespace Envoy
