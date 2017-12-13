#pragma once

#include "envoy/runtime/runtime.h"
#include "envoy/thread_local/thread_local.h"
#include "envoy/tracing/http_tracer.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/tracing/opentracing_driver_impl.h"
#include "opentracing/dynamic_load.h"

namespace Envoy {
namespace Tracing {

class DynamicOpenTracingDriver : public OpenTracingDriver {
 public:
  DynamicOpenTracingDriver(const Json::Object& config, Upstream::ClusterManager& cluster_manager,
                           Stats::Store& stats, ThreadLocal::SlotAllocator& tls,
                           Runtime::Loader& runtime,
                           const std::string& library,
                           const std::string& tracer_config);

  // Tracer::OpenTracingDriver
  const opentracing::Tracer& tracer() const override { return *tracer_; }
  bool useTracerPropagation() const override { return true; }
  bool useSingleHeaderPropagation() const override { return false; }
 private:
   opentracing::DynamicTracingLibraryHandle library_handle_;
   std::shared_ptr<opentracing::Tracer> tracer_;
};

} // Tracing
} // namespace Envoy
