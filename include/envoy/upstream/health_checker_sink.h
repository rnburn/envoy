#pragma once

#include <memory>

namespace Envoy {
namespace Upstream {

/**
 * fixfix
 */
class HealthCheckerSink {
public:
  virtual ~HealthCheckerSink() {}

  /**
   *
   */
};

typedef std::unique_ptr<HealthCheckerSink> HealthCheckerSinkPtr;

} // namespace Upstream
} // namespace Envoy
