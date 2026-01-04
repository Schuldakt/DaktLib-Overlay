#include "dakt/overlay/core/Color.hpp"

namespace dakt::overlay {

Color Color::premultiplied() const { return {r * a, g * a, b * a, a}; }

} // namespace dakt::overlay
