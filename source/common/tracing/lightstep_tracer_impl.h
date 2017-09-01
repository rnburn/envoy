#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "envoy/runtime/runtime.h"
#include "envoy/thread_local/thread_local.h"
#include "envoy/tracing/http_tracer.h"
#include "envoy/upstream/cluster_manager.h"

#include "common/http/header_map_impl.h"
#include "common/http/message_impl.h"
#include "common/tracing/opentracing_driver_impl.h"
#include "common/json/json_loader.h"

#include "lightstep/tracer.h"
#include "lightstep/transporter.h"
#include "opentracing/tracer.h"
#include "opentracing/noop.h"

namespace Envoy {
namespace Tracing {

#define LIGHTSTEP_TRACER_STATS(COUNTER)                                                            \
  COUNTER(spans_sent)                                                                              \
  COUNTER(timer_flushed)

struct LightstepTracerStats {
  LIGHTSTEP_TRACER_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * LightStep (http://lightstep.com/) provides tracing capabilities, aggregation, visualization of
 * application trace data.
 *
 * LightStepSink is for flushing data to LightStep collectors.
 */
class LightStepDriver : public OpenTracingDriver {
public:
  LightStepDriver(const Json::Object& config, Upstream::ClusterManager& cluster_manager,
                  Stats::Store& stats, ThreadLocal::SlotAllocator& tls, Runtime::Loader& runtime,
                  const lightstep::LightStepTracerOptions& options);

  // Tracer::OpenTracingDriver
  const opentracing::Tracer& tracer() const override;

  Upstream::ClusterManager& clusterManager() { return cm_; }
  Upstream::ClusterInfoConstSharedPtr cluster() { return cluster_; }
  Runtime::Loader& runtime() { return runtime_; }
  LightstepTracerStats& tracerStats() { return tracer_stats_; }

private:
  class LightStepTransporter : public lightstep::AsyncTransporter, Http::AsyncClient::Callbacks {
  public:
    explicit LightStepTransporter(LightStepDriver& driver);

    // lightstep::AsyncTransporter
    void Send(const google::protobuf::Message& request, google::protobuf::Message& response,
              void (*on_success)(void* context),
              void (*on_failure)(std::error_code error, void* context), void* context) override;

    // Http::AsyncClient::Callbacks
    void onSuccess(Http::MessagePtr&& response) override;
    void onFailure(Http::AsyncClient::FailureReason) override;

  private:
    void (*on_success_callback_)(void* context) = nullptr;
    void (*on_failure_callback_)(std::error_code error, void* context) = nullptr;
    google::protobuf::Message* active_response_ = nullptr;
    void* active_context_ = nullptr;
    LightStepDriver& driver_;
  };

  struct TlsLightStepTracer : ThreadLocal::ThreadLocalObject {
    TlsLightStepTracer(std::shared_ptr<opentracing::Tracer>&& tracer, LightStepDriver& driver);

    std::shared_ptr<opentracing::Tracer> tracer_;
    LightStepDriver& driver_;
  };

  Upstream::ClusterManager& cm_;
  Upstream::ClusterInfoConstSharedPtr cluster_;
  LightstepTracerStats tracer_stats_;
  ThreadLocal::SlotPtr tls_;
  Runtime::Loader& runtime_;
};

} // Tracing
} // namespace Envoy
