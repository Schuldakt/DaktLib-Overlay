#pragma once

#include <memory>

#include "Export.hpp"

namespace dakt::overlay {
class LayerTree;
class ISurface;

class DAKT_OVERLAY_API OverlayWindow {
public:
  OverlayWindow();
  ~OverlayWindow();

  void show();
  void hide();
  void setBounds(int x, int y, int width, int height);
  void setOpacity(float value);
  void setClickThrough(bool enabled);

  LayerTree &layerTree();
  ISurface &surface();

private:
  std::unique_ptr<LayerTree> layerTree_;
  std::unique_ptr<ISurface> surface_;
  bool visible_{false};
  bool clickThrough_{false};
  float opacity_{1.0F};
};
} // namespace dakt::overlay
