#pragma once

#include "dakt/overlay/core/Surface.hpp"

namespace dakt::overlay::platform {
class LinuxSurface final : public SoftwareSurface {
public:
  LinuxSurface();
  ~LinuxSurface() override;
};
} // namespace dakt::overlay::platform
