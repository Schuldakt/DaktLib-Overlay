#pragma once

namespace dakt::overlay {
struct Vec2 {
  float x{0.0F};
  float y{0.0F};
};

struct Rect {
  float x{0.0F};
  float y{0.0F};
  float width{0.0F};
  float height{0.0F};

  bool contains(const Vec2 &pt) const;
};
} // namespace dakt::overlay
