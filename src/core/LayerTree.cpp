#include "dakt/overlay/core/LayerTree.hpp"

#include "dakt/overlay/core/Layer.hpp"

namespace dakt::overlay {

LayerTree::LayerTree() : root_(std::make_unique<Layer>()) {}

LayerTree::~LayerTree() = default;

Layer &LayerTree::root() { return *root_; }

const Layer &LayerTree::root() const { return *root_; }

} // namespace dakt::overlay
