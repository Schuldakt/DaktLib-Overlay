#pragma once

#include <memory>
#include <vector>

#include "Color.hpp"
#include "Geometry.hpp"

namespace dakt::overlay {
class Layer {
public:
  Layer();
  ~Layer();

  void setBounds(const Rect &bounds);
  void setOpacity(float value);

  const Rect &bounds() const;
  float opacity() const;

private:
  Rect bounds_{};
  float opacity_{1.0F};
};
} // namespace dakt::overlay
