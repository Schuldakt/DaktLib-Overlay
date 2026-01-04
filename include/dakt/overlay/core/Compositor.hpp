#pragma once

namespace dakt::overlay {
class LayerTree;
class ISurface;

class Compositor {
public:
  Compositor();
  ~Compositor();

  void compose(const LayerTree &tree, ISurface &surface);
};
} // namespace dakt::overlay
