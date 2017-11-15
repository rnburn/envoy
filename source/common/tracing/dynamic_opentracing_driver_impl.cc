#include "common/tracing/dynamic_opentracing_driver_impl.h"

namespace Envoy {
namespace Tracing {

DynamicOpenTracingDriver::DynamicOpenTracingDriver(
    const Json::Object& /*config*/, Upstream::ClusterManager& /*cluster_manager*/, Stats::Store& /*stats*/,
    ThreadLocal::SlotAllocator& /*tls*/, Runtime::Loader& /*runtime*/, const std::string& /*dynamic_library*/,
    const std::string& /*config_filename*/) 
{
}

} // namespace Tracing
} // namespace Envoy
