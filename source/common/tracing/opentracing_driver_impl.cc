#include "common/tracing/opentracing_driver_impl.h"

#include <sstream>

#include "common/common/base64.h"

namespace Envoy {
namespace Tracing {

OpenTracingSpan::OpenTracingSpan(std::unique_ptr<opentracing::Span>&& span)
    : span_(std::move(span)) {
  // TODO (rnburn): check span_ != nullptr?
}

void OpenTracingSpan::finishSpan(SpanFinalizer& finalizer) {
  finalizer.finalize(*this);
  span_->Finish();
}

void OpenTracingSpan::setTag(const std::string& name, const std::string& value) {
  span_->SetTag(name, value);
}

void OpenTracingSpan::injectContext(Http::HeaderMap& request_headers) {
  std::ostringstream oss;
  span_->tracer().Inject(span_->context(), oss);
  const std::string current_span_context = oss.str();
  request_headers.insertOtSpanContext().value(
      Base64::encode(current_span_context.c_str(), current_span_context.length()));
}

SpanPtr OpenTracingSpan::spawnChild(const std::string& name, SystemTime start_time) {
  std::unique_ptr<opentracing::Span> ot_span = span_->tracer().StartSpan(
      name, {opentracing::ChildOf(&span_->context()), opentracing::StartTimestamp(start_time)});
  return SpanPtr{new OpenTracingSpan(std::move(ot_span))};
}

SpanPtr OpenTracingDriver::startSpan(Http::HeaderMap& request_headers,
                                     const std::string& operation_name, SystemTime start_time) {
  const opentracing::Tracer& tracer = this->tracer();
  std::unique_ptr<opentracing::Span> active_span;
  if (request_headers.OtSpanContext()) {
    std::string parent_context = Base64::decode(request_headers.OtSpanContext()->value().c_str());
    std::istringstream iss(parent_context);
    opentracing::expected<std::unique_ptr<opentracing::SpanContext>> parent_span_ctx_maybe =
        tracer.Extract(iss);
    std::unique_ptr<opentracing::SpanContext> parent_span_ctx;
    // TODO (rnburn): What to do if tracer.Extract fails?
    if (parent_span_ctx_maybe) {
      parent_span_ctx = std::move(*parent_span_ctx_maybe);
    }
    active_span = tracer.StartSpan(operation_name, {opentracing::ChildOf(parent_span_ctx.get()),
                                                    opentracing::StartTimestamp(start_time)});
  } else {
    active_span = tracer.StartSpan(operation_name, {opentracing::StartTimestamp(start_time)});
  }
  if (active_span == nullptr) {
    return nullptr;
  }
  return SpanPtr{new OpenTracingSpan(std::move(active_span))};
}

} // namespace Tracing
} // namespace Envoy
