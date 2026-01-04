#pragma once

namespace dakt::overlay {
struct Color {
  float r{0.0F};
  float g{0.0F};
  float b{0.0F};
  float a{1.0F};

  Color premultiplied() const;
};
} // namespace dakt::overlay
