#include "common/tracing/lightstep_tracer_impl.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <iostream>

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
    const google::protobuf::Message& request, google::protobuf::Message& response,
    void (*on_success)(void* context), void (*on_failure)(std::error_code error, void* context),
    void* context) {
  on_success_callback_ = on_success;
  on_failure_callback_ = on_failure;
  active_response_ = &response;
  active_context_ = context;

  Http::MessagePtr message =
      Grpc::Common::prepareHeaders(driver_.cluster()->name(), lightstep::CollectorServiceFullName(),
                                   lightstep::CollectorMethodName());
  message->body() = Grpc::Common::serializeBody(request);

  uint64_t timeout =
      driver_.runtime().snapshot().getInteger("tracing.lightstep.request_timeout", 5000U);
  driver_.clusterManager()
      .httpAsyncClientForCluster(driver_.cluster()->name())
      .send(std::move(message), *this, std::chrono::milliseconds(timeout));
}

void LightStepDriver::LightStepTransporter::onSuccess(Http::MessagePtr&& response) {
  try {
    Grpc::Common::validateResponse(*response);

    Grpc::Common::chargeStat(*driver_.cluster(), lightstep::CollectorServiceFullName(),
                             lightstep::CollectorMethodName(), true);
    if (!active_response_->ParseFromString(response->bodyAsString())) {
      throw EnvoyException("Failed to parse LightStep collector response");
    }
    on_success_callback_(active_context_);
  } catch (const Grpc::Exception& ex) {
    Grpc::Common::chargeStat(*driver_.cluster(), lightstep::CollectorServiceFullName(),
                             lightstep::CollectorMethodName(), false);
    on_failure_callback_(std::error_code(), active_context_);
  }
}

void LightStepDriver::LightStepTransporter::onFailure(Http::AsyncClient::FailureReason) {
  Grpc::Common::chargeStat(*driver_.cluster(), lightstep::CollectorServiceFullName(),
                           lightstep::CollectorMethodName(), false);
  on_failure_callback_(std::error_code(), active_context_);
}

LightStepDriver::TlsLightStepTracer::TlsLightStepTracer(
    std::shared_ptr<lightstep::LightStepTracer>&& tracer, LightStepDriver& driver,
    Event::Dispatcher& dispatcher)
    : tracer_(std::move(tracer)), driver_(driver) {
  flush_timer_ = dispatcher.createTimer([this]() -> void {
    driver_.tracerStats().timer_flushed_.inc();
    tracer_->Flush();
    enableTimer();
  });

  enableTimer();
}

const opentracing::Tracer& LightStepDriver::TlsLightStepTracer::tracer() const { return *tracer_; }

void LightStepDriver::TlsLightStepTracer::enableTimer() {
  uint64_t flush_interval =
      driver_.runtime().snapshot().getInteger("tracing.lightstep.flush_interval_ms", 1000U);
  flush_timer_->enableTimer(std::chrono::milliseconds(flush_interval));
}

LightStepDriver::LightStepDriver(const Json::Object& config,
                                 Upstream::ClusterManager& cluster_manager, Stats::Store& stats,
                                 ThreadLocal::SlotAllocator& tls, Runtime::Loader& runtime,
                                 std::unique_ptr<lightstep::LightStepTracerOptions>&& options)
    : cm_(cluster_manager), tracer_stats_{LIGHTSTEP_TRACER_STATS(
                                POOL_COUNTER_PREFIX(stats, "tracing.lightstep."))},
      tls_(tls.allocateSlot()), runtime_(runtime), options_(std::move(options)) {
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

  tls_->set([this](Event::Dispatcher& dispatcher) -> ThreadLocal::ThreadLocalObjectSharedPtr {
    lightstep::LightStepTracerOptions tls_options;
    tls_options.access_token = options_->access_token;
    tls_options.component_name = options_->component_name;
    tls_options.use_thread = false;
    tls_options.transporter.reset(new LightStepTransporter{*this});
    std::shared_ptr<lightstep::LightStepTracer> tracer =
        lightstep::MakeLightStepTracer(std::move(tls_options));

    return ThreadLocal::ThreadLocalObjectSharedPtr{
        new TlsLightStepTracer(std::move(tracer), *this, dispatcher)};
  });
}

const opentracing::Tracer& LightStepDriver::tracer() const {
  return tls_->getTyped<TlsLightStepTracer>().tracer();
}

} // namespace Tracing
} // namespace Envoy
