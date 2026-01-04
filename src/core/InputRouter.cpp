#include "dakt/overlay/core/InputRouter.hpp"

#include "dakt/overlay/core/Layer.hpp"

namespace dakt::overlay {

InputRouter::InputRouter() = default;
InputRouter::~InputRouter() = default;

void InputRouter::setRoot(Layer *root) { root_ = root; }

bool InputRouter::hitTest(const Vec2 &pt) const {
  return root_ ? root_->bounds().contains(pt) : false;
}

} // namespace dakt::overlay
