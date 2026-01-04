#include "dakt/overlay/core/Layer.hpp"

namespace dakt::overlay {

Layer::Layer() = default;
Layer::~Layer() = default;

void Layer::setBounds(const Rect &bounds) { bounds_ = bounds; }

void Layer::setOpacity(float value) { opacity_ = value; }

const Rect &Layer::bounds() const { return bounds_; }

float Layer::opacity() const { return opacity_; }

} // namespace dakt::overlay
