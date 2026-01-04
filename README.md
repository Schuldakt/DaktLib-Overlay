# DaktLib-Overlay

EAC-safe overlay compositor built with plain OS windows and no injection/hooks. Targets Windows, Linux (X11/Wayland), and macOS with a platform-neutral core and a C API meant for ClangSharp or other FFI generators.

## Current Status
- CMake scaffolding, export macros, and core namespaces are in place (C++23).
- Core primitives exist: `OverlayManager`, `OverlayWindow`, `Layer`/`LayerTree`, math helpers (`Rect`, `Vec2`, `Color`), and a stub `Painter`.
- C API covers manager/window handles, create/destroy, bounds, opacity, and click-through toggles.
- Surfaces and platform backends are stubbed; there is no real rendering, compositing, or OS window wiring yet.
- Logging/error taxonomy, draw primitives, batching, and platform features are pending (see [TODO.md](TODO.md)).

## Build
Prereqs: CMake 4.2+, a C++23 compiler. On macOS, Objective-C++ is auto-enabled for the platform layer.

### Configure
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

### Build
```sh
cmake --build build --config RelWithDebInfo
```
(Multi-config generators like Visual Studio honor the `--config` flag.)

### Install (optional)
```sh
cmake --install build --prefix "$PWD/out"
```

### Tests
A test suite is not wired up yet. Add tests under `tests/` and register them with CTest when available.

## Usage
### C++ API
```cpp
#include <dakt/overlay/core/OverlayManager.hpp>

int main() {
  dakt::overlay::OverlayManager mgr;
  auto window = mgr.createWindow();
  window->setBounds(0, 0, 800, 600);
  window->setOpacity(0.9F);
  window->setClickThrough(true);
  window->show();
  // Draw/present hooks are still stubs.
  return 0;
}
```

### C API
```c
#include <dakt/overlay/c_api.h>

int main(void) {
  DaktOverlayManagerHandle mgr = dakt_overlay_create();

  DaktOverlayConfig cfg = {
      .x = 0,
      .y = 0,
      .width = 800,
      .height = 600,
      .opacity = 0.9F,
      .clickThrough = 1,
  };

  DaktOverlayWindowHandle wnd = dakt_overlay_create_window(mgr, &cfg);
  dakt_overlay_show(wnd);
  // Present/draw entrypoints are not implemented yet.

  dakt_overlay_destroy_window(wnd);
  dakt_overlay_destroy(mgr);
  return 0;
}
```

## Project Layout
- Public headers live under `include/dakt/overlay/` (core + per-platform shims).
- Source implementations live under `src/` with `platform/<os>/` directories.
- High-level architecture notes: [ARCHITECTURE.md](ARCHITECTURE.md).

## Roadmap
The staged plan is tracked in [TODO.md](TODO.md). Short-term priorities:
- Implement real surfaces and OS window wiring (Windows first).
- Flesh out painter batching, primitives, and clipping.
- Define error/logging strategy and wire it through the C API.
- Add tests (layer tree, hit-testing) and platform samples.

## Contributing
Keep changes minimal-dependency and platform-neutral in `core/`; isolate OS-specific work under `platform/<os>/`. Please update [TODO.md](TODO.md) and [ARCHITECTURE.md](ARCHITECTURE.md) when you add new capabilities.
