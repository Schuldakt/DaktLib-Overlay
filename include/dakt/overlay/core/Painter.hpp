#pragma once

#include <vector>

#include "Color.hpp"
#include "Geometry.hpp"

namespace dakt::overlay {
class ISurface;

struct DrawCommand {
  Rect bounds;
  Color color;
};

class Painter {
public:
  Painter();
  ~Painter();

  void begin();
  void drawRect(const Rect &rect, const Color &color);
  void end(ISurface &surface);

private:
  std::vector<DrawCommand> commands_;
};
} // namespace dakt::overlay
