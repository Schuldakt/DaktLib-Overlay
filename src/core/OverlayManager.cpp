#include "dakt/overlay/core/OverlayManager.hpp"

#include <algorithm>
#include <memory>

#include "dakt/overlay/core/OverlayWindow.hpp"

namespace dakt::overlay {

OverlayManager::OverlayManager() = default;
OverlayManager::~OverlayManager() = default;

std::shared_ptr<OverlayWindow> OverlayManager::createWindow() {
  auto window = std::make_shared<OverlayWindow>();
  windows_.push_back(window);
  return window;
}

void OverlayManager::destroyWindow(
    const std::shared_ptr<OverlayWindow> &window) {
  windows_.erase(std::remove(windows_.begin(), windows_.end(), window),
                 windows_.end());
}

const std::vector<std::shared_ptr<OverlayWindow>> &
OverlayManager::windows() const {
  return windows_;
}

} // namespace dakt::overlay
