#include "dakt/overlay/core/Compositor.hpp"

#include "dakt/overlay/core/LayerTree.hpp"
#include "dakt/overlay/core/Surface.hpp"

namespace dakt::overlay {

Compositor::Compositor() = default;
Compositor::~Compositor() = default;

void Compositor::compose(const LayerTree &, ISurface &) {}

} // namespace dakt::overlay
