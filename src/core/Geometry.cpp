#include "dakt/overlay/core/Geometry.hpp"

namespace dakt::overlay {

bool Rect::contains(const Vec2 &pt) const {
  const bool withinX = pt.x >= x && pt.x <= (x + width);
  const bool withinY = pt.y >= y && pt.y <= (y + height);
  return withinX && withinY;
}

} // namespace dakt::overlay
