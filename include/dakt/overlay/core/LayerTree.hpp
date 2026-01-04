#pragma once

#include <memory>

namespace dakt::overlay {
class Layer;

class LayerTree {
public:
  LayerTree();
  ~LayerTree();

  Layer &root();
  const Layer &root() const;

private:
  std::unique_ptr<Layer> root_;
};
} // namespace dakt::overlay
