#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

#include "common/network/address_impl.h"

#include "api/base.pb.h"
#include "api/bootstrap.pb.h"
#include "api/cds.pb.h"
#include "api/filter/http/http_connection_manager.pb.h"
#include "api/protocol.pb.h"
#include "api/rds.pb.h"

namespace Envoy {

class ConfigHelper {
public:
  // Set up basic config, using the specified IpVersion for all connections: listeners, upstream,
  // and admin connections.
  ConfigHelper(const Network::Address::IpVersion version);

  typedef std::function<void(envoy::api::v2::Bootstrap&)> ConfigModifierFunction;
  typedef std::function<void(envoy::api::v2::filter::http::HttpConnectionManager&)>
      HttpModifierFunction;

  // A string for a basic buffer filter, which can be used with addFilter()
  static const std::string DEFAULT_BUFFER_FILTER;
  // a string for a health check filter which can be used with addFilter()
  static const std::string DEFAULT_HEALTH_CHECK_FILTER;

  // Run the final config modifiers, and then set the upstream ports based on upstream connections.
  // This is the last operation run on |bootstrap_| before it is handed to Envoy.
  // Ports are assigned by looping through clusters, hosts, and addresses in the
  // order they are stored in |bootstrap_|
  void finalize(const std::vector<uint32_t>& ports);

  // Set source_address in the bootstrap bind config.
  void setSourceAddress(const std::string& address_string);

  // Overwrite the first host and route for the primary listener.
  void setDefaultHostAndRoute(const std::string& host, const std::string& route);

  // Sets byte limits on upstream and downstream connections.
  void setBufferLimits(uint32_t upstream_buffer_limit, uint32_t downstream_buffer_limit);

  // Set the connect timeout on upstream connections.
  void setConnectTimeout(std::chrono::milliseconds timeout);

  // Add an additional route to the configuration.
  void addRoute(
      const std::string& host, const std::string& route, const std::string& cluster,
      envoy::api::v2::VirtualHost::TlsRequirementType type = envoy::api::v2::VirtualHost::NONE);

  // Add an HTTP filter prior to existing filters.
  void addFilter(const std::string& filter_yaml);

  // Sets the client codec to the specified type.
  void setClientCodec(envoy::api::v2::filter::http::HttpConnectionManager::CodecType type);

  // Add the default SSL configuration.
  void addSslConfig();

  // Allows callers to do their own modification to |bootstrap_| which will be
  // applied just before ports are modified in finalize().
  void addConfigModifier(ConfigModifierFunction function);

  // Allows callers to easily modify the HttpConnectionManager configuration.
  // Mofidiers will be applied just before ports are modified in finalize
  void addConfigModifier(HttpModifierFunction function);

  // Return the bootstrap configuration for hand-off to Envoy.
  const envoy::api::v2::Bootstrap& bootstrap() { return bootstrap_; }

private:
  // Load the first HCM struct from the first listener into a parsed proto.
  void loadHttpConnectionManager(envoy::api::v2::filter::http::HttpConnectionManager& hcm);
  // Stick the contents of the procided HCM proto and stuff them into the first HCM
  // struct of the first listener.
  void storeHttpConnectionManager(const envoy::api::v2::filter::http::HttpConnectionManager& hcm);

  // The bootstrap proto Envoy will start up with.
  envoy::api::v2::Bootstrap bootstrap_;

  // The config modifiers added via addConfigModifier() which will be applied in finalize()
  std::vector<ConfigModifierFunction> config_modifiers_;

  // Track if the connect timeout has been set (to avoid clobbering a custom setting with the
  // default).
  bool connect_timeout_set_{false};

  // A sanity check guard to make sure config is not modified after handing it to Envoy.
  bool finalized_{false};
};

} // namespace Envoy
