#include "common/tracing/lightstep_tracer_impl.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

#include "common/common/base64.h"
#include "common/grpc/common.h"
#include "common/http/message_impl.h"
#include "common/tracing/http_tracer_impl.h"

#include "spdlog/spdlog.h"

namespace Envoy {
namespace Tracing {

LightStepDriver::LightStepTransporter::LightStepTransporter(LightStepDriver& driver)
  : driver_(driver) {}

void LightStepDriver::LightStepTransporter::Send(
    const google::protobuf::Message& /*request*/, google::protobuf::Message& response,
    void (*on_success)(void* context), void (*on_failure)(std::error_code error, void* context),
    void* context) {
  on_success_callback_ = on_success;
  on_failure_callback_ = on_failure;
  active_response_ = &response;
  active_context_ = context;
}

void LightStepDriver::LightStepTransporter::onSuccess(Http::MessagePtr&&) {}

void LightStepDriver::LightStepTransporter::onFailure(Http::AsyncClient::FailureReason) {}

LightStepDriver::TlsLightStepTracer::TlsLightStepTracer(
    std::shared_ptr<opentracing::Tracer>&& tracer, LightStepDriver& driver)
    : tracer_(std::move(tracer)), driver_(driver) {}

LightStepDriver::LightStepDriver(const Json::Object& config,
                                 Upstream::ClusterManager& cluster_manager, Stats::Store& stats,
                                 ThreadLocal::SlotAllocator& tls, Runtime::Loader& runtime,
                                 const lightstep::LightStepTracerOptions& /*options*/)
    : cm_(cluster_manager), tracer_stats_{LIGHTSTEP_TRACER_STATS(
                                POOL_COUNTER_PREFIX(stats, "tracing.lightstep."))},
      tls_(tls.allocateSlot()), runtime_(runtime) {
  Upstream::ThreadLocalCluster* cluster = cm_.get(config.getString("collector_cluster"));
  if (!cluster) {
    throw EnvoyException(fmt::format("{} collector cluster is not defined on cluster manager level",
                                     config.getString("collector_cluster")));
  }
  cluster_ = cluster->info();

  if (!(cluster_->features() & Upstream::ClusterInfo::Features::HTTP2)) {
    throw EnvoyException(
        fmt::format("{} collector cluster must support http2 for gRPC calls", cluster_->name()));
  }

  tls_->set([this](Event::Dispatcher & /*dispatcher*/) -> ThreadLocal::ThreadLocalObjectSharedPtr {
    std::shared_ptr<opentracing::Tracer> tracer = opentracing::MakeNoopTracer();

    return ThreadLocal::ThreadLocalObjectSharedPtr{
        new TlsLightStepTracer(std::move(tracer), *this)};
  });
}

const opentracing::Tracer& LightStepDriver::tracer() const {
  return *tls_->getTyped<TlsLightStepTracer>().tracer_;
}

} // namespace Tracing
} // namespace Envoy
