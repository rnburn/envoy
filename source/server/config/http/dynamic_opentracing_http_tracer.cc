#include "server/config/http/dynamic_opentracing_http_tracer.h"


#include <string>

#include "envoy/registry/registry.h"

#include "common/common/utility.h"
#include "common/config/well_known_names.h"
#include "common/tracing/http_tracer_impl.h"
#include "common/tracing/dynamic_opentracing_driver_impl.h"

namespace Envoy {
namespace Server {
namespace Configuration {

Tracing::HttpTracerPtr
DynamicOpenTracingHttpTracerFactory::createHttpTracer(const Json::Object& /*json_config*/,
                                             Server::Instance& /*server*/,
                                             Upstream::ClusterManager& /*cluster_manager*/) {

  std::cerr << "ArfArf" << std::endl;
  return nullptr;
  /* std::unique_ptr<lightstep::LightStepTracerOptions> opts(new lightstep::LightStepTracerOptions()); */
  /* opts->access_token = server.api().fileReadToEnd(json_config.getString("access_token_file")); */
  /* StringUtil::rtrim(opts->access_token); */
  /* opts->component_name = server.localInfo().clusterName(); */

  /* Tracing::DriverPtr lightstep_driver( */
  /*     new Tracing::LightStepDriver(json_config, cluster_manager, server.stats(), */
  /*                                  server.threadLocal(), server.runtime(), std::move(opts))); */
  /* return Tracing::HttpTracerPtr( */
  /*     new Tracing::HttpTracerImpl(std::move(lightstep_driver), server.localInfo())); */
}

std::string DynamicOpenTracingHttpTracerFactory::name() { return Config::HttpTracerNames::get().DYNAMIC; }

/**
 * Static registration for the lightstep http tracer. @see RegisterFactory.
 */
static Registry::RegisterFactory<DynamicOpenTracingHttpTracerFactory, HttpTracerFactory> register_;

} // namespace Configuration
} // namespace Server
} // namespace Envoy
