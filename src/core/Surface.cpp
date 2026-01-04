#include "dakt/overlay/core/Surface.hpp"

namespace dakt::overlay {

SoftwareSurface::SoftwareSurface() = default;
SoftwareSurface::~SoftwareSurface() = default;

void SoftwareSurface::resize(int, int) {}

void SoftwareSurface::present() {}

void SoftwareSurface::setOpacity(float) {}

void SoftwareSurface::setHitTest(bool) {}

} // namespace dakt::overlay
