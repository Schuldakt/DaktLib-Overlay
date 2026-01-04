#include "dakt/overlay/core/Painter.hpp"

#include "dakt/overlay/core/Surface.hpp"

namespace dakt::overlay {

Painter::Painter() = default;
Painter::~Painter() = default;

void Painter::begin() { commands_.clear(); }

void Painter::drawRect(const Rect &rect, const Color &color) {
  commands_.push_back({rect, color});
}

void Painter::end(ISurface &) { commands_.clear(); }

} // namespace dakt::overlay
