#pragma once

#include <memory>

#include "Geometry.hpp"

namespace dakt::overlay {
class Layer;

class InputRouter {
public:
  InputRouter();
  ~InputRouter();

  void setRoot(Layer *root);
  bool hitTest(const Vec2 &pt) const;

private:
  Layer *root_{nullptr};
};
} // namespace dakt::overlay
