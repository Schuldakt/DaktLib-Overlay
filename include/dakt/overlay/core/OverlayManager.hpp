#pragma once

#include <memory>
#include <vector>

#include "Export.hpp"

namespace dakt::overlay {
class OverlayWindow;

class DAKT_OVERLAY_API OverlayManager {
public:
  OverlayManager();
  ~OverlayManager();

  std::shared_ptr<OverlayWindow> createWindow();
  void destroyWindow(const std::shared_ptr<OverlayWindow> &window);
  const std::vector<std::shared_ptr<OverlayWindow>> &windows() const;

private:
  std::vector<std::shared_ptr<OverlayWindow>> windows_;
};
} // namespace dakt::overlay
