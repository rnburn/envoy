#include "common/tracing/opentracing_driver_impl.h"

#include <sstream>

#include "common/common/base64.h"

namespace Envoy {
namespace Tracing {

namespace {
class OpenTracingHTTPHeadersWriter : public opentracing::HTTPHeadersWriter {
public:
  explicit OpenTracingHTTPHeadersWriter(Http::HeaderMap& request_headers)
      : request_headers_(request_headers) {}

  // opentracing::HTTPHeadersWriter
  opentracing::expected<void> Set(opentracing::string_view key,
                                  opentracing::string_view value) const override {
    request_headers_.addCopy(Http::LowerCaseString{key}, value);
    return {};
  }

private:
  Http::HeaderMap& request_headers_;
};
} // namespace

OpenTracingSpan::OpenTracingSpan(std::unique_ptr<opentracing::Span>&& span)
    : span_(std::move(span)) {}

void OpenTracingSpan::finishSpan(SpanFinalizer& finalizer) {
  finalizer.finalize(*this);
  span_->Finish();
}

void OpenTracingSpan::setOperation(const std::string& operation) {
  span_->SetOperationName(operation);
}

void OpenTracingSpan::setTag(const std::string& name, const std::string& value) {
  span_->SetTag(name, value);
}

void OpenTracingSpan::injectContext(Http::HeaderMap& request_headers) {
  // Inject the span context using Envoy's single-header format.
  std::ostringstream oss;
  opentracing::expected<void> was_successful = span_->tracer().Inject(span_->context(), oss);
  if (!was_successful) {
    ENVOY_LOG(warn, "Failed to inject span context: {}", was_successful.error().message());
    return;
  }
  const std::string current_span_context = oss.str();
  request_headers.insertOtSpanContext().value(
      Base64::encode(current_span_context.c_str(), current_span_context.length()));

  // Also, inject the context using the tracer's standard HTTP header format.
  const OpenTracingHTTPHeadersWriter writer{request_headers};
  was_successful = span_->tracer().Inject(span_->context(), writer);
  if (!was_successful) {
    ENVOY_LOG(warn, "Failed to inject span context: {}", was_successful.error().message());
    return;
  }
}

SpanPtr OpenTracingSpan::spawnChild(const Config&, const std::string& name, SystemTime start_time) {
  std::unique_ptr<opentracing::Span> ot_span = span_->tracer().StartSpan(
      name, {opentracing::ChildOf(&span_->context()), opentracing::StartTimestamp(start_time)});
  if (ot_span == nullptr) {
    return nullptr;
  }
  return SpanPtr{new OpenTracingSpan(std::move(ot_span))};
}

SpanPtr OpenTracingDriver::startSpan(const Config&, Http::HeaderMap& request_headers,
                                     const std::string& operation_name, SystemTime start_time) {
  const opentracing::Tracer& tracer = this->tracer();
  std::unique_ptr<opentracing::Span> active_span;
  if (request_headers.OtSpanContext()) {
    std::string parent_context = Base64::decode(request_headers.OtSpanContext()->value().c_str());
    std::istringstream iss(parent_context);
    opentracing::expected<std::unique_ptr<opentracing::SpanContext>> parent_span_ctx_maybe =
        tracer.Extract(iss);
    std::unique_ptr<opentracing::SpanContext> parent_span_ctx;
    if (parent_span_ctx_maybe) {
      parent_span_ctx = std::move(*parent_span_ctx_maybe);
    } else {
      ENVOY_LOG(warn, "Failed to extract span context: {}",
                parent_span_ctx_maybe.error().message());
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
