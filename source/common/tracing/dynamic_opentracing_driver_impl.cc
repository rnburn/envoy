#include "common/tracing/dynamic_opentracing_driver_impl.h"

#include <dlfcn.h>

#include "common/common/assert.h"

namespace Envoy {
namespace Tracing {

namespace {
struct DLHandle {
  void* handle = nullptr;

  ~DLHandle() {
    if (handle != nullptr) {
      dlclose(handle);
    }
  }
};
}  // anonymous namespace

DynamicOpenTracingDriver::DynamicOpenTracingDriver(
    const Json::Object& /*config*/, Upstream::ClusterManager& /*cluster_manager*/, Stats::Store& /*stats*/,
    ThreadLocal::SlotAllocator& /*tls*/, Runtime::Loader& /*runtime*/, const std::string& library,
    const std::string& tracer_config) 
{
  loadTracer(library, tracer_config);
}

void DynamicOpenTracingDriver::loadTracer(const std::string& library,
                                          const std::string& tracer_config) {
  DLHandle handle;
  handle.handle = dlopen(library.c_str(), RTLD_NOW);
  if (handle.handle == nullptr) {
    throw EnvoyException(fmt::format("failed to load tracing library '{}'", library));
  }

  // deduce the name of the function to load from the library name
  size_t name_first = 0;
  size_t name_last = library.size();
  const size_t last_slash = library.find_last_of('/');
  if (last_slash != std::string::npos)
    name_first = last_slash+1;
  const size_t dot_position = library.find('.', name_first);
  if (dot_position != std::string::npos)
    name_last = dot_position;
  auto function_name =
      fmt::format("make_{}_tracer", library.substr(name_first, name_last - name_first));

  dlerror(); // Clear any existing error.
  auto make_tracer = reinterpret_cast<int (*)(const char*, void*, void*)>(
      dlsym(handle.handle, function_name.c_str()));
  if (make_tracer == nullptr) {
    throw EnvoyException(
        fmt::format("failed to find tracing library function '{}'", function_name));
  }
  std::string error_message;
  int rcode = make_tracer(tracer_config.c_str(), static_cast<void*>(&tracer_),
                          static_cast<void*>(&error_message));
  if (rcode != 0) {
    throw EnvoyException(
        fmt::format("failed to construct tracer: '{}'", error_message));
  }
  RELEASE_ASSERT(tracer_ != nullptr);
}

} // namespace Tracing
} // namespace Envoy
