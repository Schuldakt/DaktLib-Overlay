#include "dakt/overlay/core/OverlayWindow.hpp"

#include <utility>

#include "dakt/overlay/core/LayerTree.hpp"
#include "dakt/overlay/core/Surface.hpp"

namespace dakt::overlay {

OverlayWindow::OverlayWindow()
    : layerTree_(std::make_unique<LayerTree>()),
      surface_(std::make_unique<SoftwareSurface>()) {}

OverlayWindow::~OverlayWindow() = default;

void OverlayWindow::show() { visible_ = true; }

void OverlayWindow::hide() { visible_ = false; }

void OverlayWindow::setBounds(int, int, int width, int height) {
  surface_->resize(width, height);
}

void OverlayWindow::setOpacity(float value) {
  opacity_ = value;
  surface_->setOpacity(value);
}

void OverlayWindow::setClickThrough(bool enabled) {
  clickThrough_ = enabled;
  surface_->setHitTest(!enabled);
}

LayerTree &OverlayWindow::layerTree() { return *layerTree_; }

ISurface &OverlayWindow::surface() { return *surface_; }

} // namespace dakt::overlay
