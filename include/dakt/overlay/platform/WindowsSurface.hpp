#pragma once

#include "dakt/overlay/core/Surface.hpp"

namespace dakt::overlay::platform {
class WindowsSurface final : public SoftwareSurface {
public:
  WindowsSurface();
  ~WindowsSurface() override;
};
} // namespace dakt::overlay::platform
