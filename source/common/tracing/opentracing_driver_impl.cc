#include "common/tracing/opentracing_driver_impl.h"

namespace Envoy {
namespace Tracing {

OpenTracingSpan::OpenTracingSpan(std::unique_ptr<opentracing::Span>&& span)
    : span_(std::move(span)) {}

void OpenTracingSpan::finishSpan(SpanFinalizer& finalizer) {
  finalizer.finalize(*this);
  span_->Finish();
}

void OpenTracingSpan::setTag(const std::string& name, const std::string& value) {
  span_->SetTag(name, value);
}

void OpenTracingSpan::injectContext(Http::HeaderMap& /*request_headers*/) {}

SpanPtr OpenTracingSpan::spawnChild(const std::string& /*name*/, SystemTime /*start_time*/) {
  return nullptr;
}

SpanPtr OpenTracingDriver::startSpan(Http::HeaderMap& /*request_headers*/,
                                     const std::string& /*operation_name*/,
                                     SystemTime /*start_time*/) {
  return nullptr;
}

} // namespace Tracing
} // namespace Envoy
