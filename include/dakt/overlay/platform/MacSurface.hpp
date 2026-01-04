#pragma once

#include "dakt/overlay/core/Surface.hpp"

namespace dakt::overlay::platform {
class MacSurface final : public SoftwareSurface {
public:
  MacSurface();
  ~MacSurface() override;
};
} // namespace dakt::overlay::platform
