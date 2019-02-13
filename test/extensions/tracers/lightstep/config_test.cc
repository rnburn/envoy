#include "extensions/tracers/lightstep/config.h"

#include "test/mocks/server/mocks.h"
#include "test/test_common/test_base.h"

#include "gmock/gmock.h"

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace Envoy {
namespace Extensions {
namespace Tracers {
namespace Lightstep {

TEST(LightstepTracerConfigTest, LightstepHttpTracer) {
  NiceMock<Server::MockInstance> server;
  EXPECT_CALL(server.cluster_manager_, get("fake_cluster"))
      .WillRepeatedly(Return(&server.cluster_manager_.thread_local_cluster_));
  ON_CALL(*server.cluster_manager_.thread_local_cluster_.cluster_.info_, features())
      .WillByDefault(Return(Upstream::ClusterInfo::Features::HTTP2));

  const std::string yaml_string = R"EOF(
  http:
    name: envoy.lightstep
    config:
      collector_cluster: fake_cluster
      access_token_file: fake_file
   )EOF";
  envoy::config::trace::v2::Tracing configuration;
  MessageUtil::loadFromYaml(yaml_string, configuration);

  LightstepTracerFactory factory;
  auto message = Config::Utility::translateToFactoryConfig(configuration.http(), factory);
  Tracing::DriverPtr lightstep_driver = factory.createDriver(*message, server);
  EXPECT_NE(nullptr, lightstep_driver);
}

} // namespace Lightstep
} // namespace Tracers
} // namespace Extensions
} // namespace Envoy
