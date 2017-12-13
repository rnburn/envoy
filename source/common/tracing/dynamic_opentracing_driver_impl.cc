#include "common/tracing/dynamic_opentracing_driver_impl.h"

#include <dlfcn.h>

#include "common/common/assert.h"

namespace Envoy {
namespace Tracing {

DynamicOpenTracingDriver::DynamicOpenTracingDriver(
    const Json::Object& /*config*/, Upstream::ClusterManager& /*cluster_manager*/, Stats::Store& /*stats*/,
    ThreadLocal::SlotAllocator& /*tls*/, Runtime::Loader& /*runtime*/, const std::string& library,
    const std::string& tracer_config) 
{
  std::string error_message;
  opentracing::expected<opentracing::DynamicTracingLibraryHandle> library_handle_maybe =
      opentracing::dynamically_load_tracing_library(library.c_str(), error_message);
  if (!library_handle_maybe) {
    if (error_message.empty()) {
      throw EnvoyException{fmt::format("{}", library_handle_maybe.error().message())};
    } else {
      throw EnvoyException{
          fmt::format("{}: {}", library_handle_maybe.error().message(), error_message)};
    }
  }
  library_handle_ = std::move(*library_handle_maybe);

  opentracing::expected<std::shared_ptr<opentracing::Tracer>> tracer_maybe =
      library_handle_.tracer_factory().MakeTracer(tracer_config.c_str(), error_message);
  if (!tracer_maybe) {
    if (error_message.empty()) {
      throw EnvoyException{fmt::format("{}", tracer_maybe.error().message())};
    } else {
      throw EnvoyException{fmt::format("{}: {}", tracer_maybe.error().message(), error_message)};
    }
  }
  tracer_ = std::move(*tracer_maybe);
  RELEASE_ASSERT(tracer_ != nullptr);
}

} // namespace Tracing
} // namespace Envoy
