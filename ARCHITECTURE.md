# DaktLib-Overlay Architecture

> EAC-compliant overlay compositor for out-of-process overlays. Uses OS-level windows only; no process injection or graphics API hooking.

## Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      Application Layer                       │
├─────────────────────────────────────────────────────────────┤
│                  OverlayManager / C API                      │
├─────────────────────────────────────────────────────────────┤
│   Layer Tree   │   Input Router   │   Composition            │
├─────────────────────────────────────────────────────────────┤
│       Surface Abstraction (ISurface) / Render Pipeline       │
├───────────────┬───────────────────┬───────────────┬─────────┤
│  Windows      │      Linux        │    macOS      │ Shared  │
│ Layered Win   │ X11 + Composite   │  NSWindow +   │  Core   │
│ DirectComp    │  Wayland via PW   │  CoreAnimation│         │
└───────────────┴───────────────────┴───────────────┴─────────┘
```

## Compliance
- ❌ No DLL injection, no API hooking, no game-process memory access.
- ✅ Separate top-level windows with transparency (per-OS composition).
- ✅ Input optional (passthrough when not focused).

## Namespace Structure

```cpp
namespace dakt::overlay {
    // Core
    class OverlayManager;
    class OverlayWindow;
    class Layer;
    class LayerTree;
    class InputRouter;

    // Surfaces
    class ISurface;
    class SwapchainSurface; // For GPU-backed rendering
    class SoftwareSurface;  // CPU fallback

    // Composition
    class Compositor;
    struct BlendState;

    // Geometry & Text
    class Painter;
    struct Vertex;
    struct Color;

    // Platform backends
    namespace platform {
        class WindowsSurface;   // WS_EX_LAYERED + DirectComposition
        class LinuxSurface;     // X11 composite or Wayland (PipeWire stream)
        class MacSurface;       // NSWindow + CoreAnimation
    }
}
```

## Modular Directory Structure

```
DaktLib-Overlay/
├── include/dakt/overlay/
│   ├── core/
│   │   ├── OverlayManager.hpp       # Manager + window orchestration
│   │   ├── OverlayWindow.hpp        # Window wrapper
│   │   ├── Layer.hpp                # Layer primitive
│   │   ├── LayerTree.hpp            # Tree traversal + z-order
│   │   ├── InputRouter.hpp          # Hit-test routing
│   │   ├── Painter.hpp              # Draw list / batching
│   │   ├── Surface.hpp              # ISurface + swaps/software
│   │   ├── Compositor.hpp           # Composition entry
│   │   ├── Geometry.hpp             # Rect/vec helpers
│   │   ├── Color.hpp                # Premultiplied color utilities
│   │   └── Export.hpp               # Visibility macro
│   ├── platform/
│   │   ├── WindowsSurface.hpp       # WS_EX_LAYERED + DirectComposition
│   │   ├── LinuxSurface.hpp         # X11/Wayland abstraction
│   │   └── MacSurface.hpp           # NSWindow/CoreAnimation
│   └── c_api.h                      # C ABI surface
├── src/
│   ├── core/
│   │   ├── OverlayManager.cpp
│   │   ├── OverlayWindow.cpp
│   │   ├── Layer.cpp
│   │   ├── LayerTree.cpp
│   │   ├── InputRouter.cpp
│   │   ├── Painter.cpp
│   │   ├── Surface.cpp
│   │   ├── Compositor.cpp
│   │   ├── Geometry.cpp
│   │   └── Color.cpp
│   ├── platform/
│   │   ├── windows/WindowsSurface.cpp
│   │   ├── linux/LinuxSurface.cpp
│   │   └── mac/MacSurface.mm
│   └── c_api.cpp
├── tests/
├── CMakeLists.txt
├── ARCHITECTURE.md
└── TODO.md
```

### Modularization Notes
- `core/` isolates platform-agnostic logic so new platforms can be added without touching overlays/painter APIs.
- `platform/<os>/` keeps OS-specific windowing and surface code contained; shared interfaces stay under `platform/` headers.
- `c_api.h` remains flat for ClangSharp friendliness; it includes only public handles and POD structs.
- Additional features (new blend modes, surfaces, or primitives) plug into `core` without layout churn.

## Core Components

### OverlayManager
- Creates/destroys `OverlayWindow` instances.
- Manages z-order, DPI awareness, multi-monitor placement.
- Controls input passthrough vs capture (click-through flag).

### OverlayWindow
- Represents a top-level translucent window.
- Owns a `LayerTree` and `Surface`.
- Provides API for size/position, opacity, visibility, focus behavior.

### Layer & LayerTree
- `Layer` = drawable node with transform, opacity, visibility, clip rect.
- `LayerTree` composes layers into `DrawList` consumed by `Painter`.
- Supports primitives: rect, rounded rect, circle, line, polyline, polygon, image, text.

### InputRouter
- Hit-testing on layer bounds; delivers hover/click/scroll/key events.
- Can be disabled for full click-through overlay.
- Optional focus ring rendering.

### Painter
- High-level drawing API on top of `ISurface` (can reuse DaktLib-GUI DrawList).
- Batches commands, manages font/text rendering via shared font atlas.

### Surfaces (ISurface)
- Abstracts platform-specific drawing target.
- Methods: `resize`, `present`, `updateTexture`, `setOpacity`, `setHitTest(bool)`.
- Swapchain-backed where available; software fallback for minimal dependencies.

### Composition & Blending
- Blend states: premultiplied alpha, additive, multiply.
- Clip stack for per-layer clipping.
- Optional color space conversion (sRGB).

## Platform Implementations

### Windows
- WS_EX_LAYERED with per-pixel alpha (UpdateLayeredWindow) for broad compatibility.
- DirectComposition path for better performance (win10+), still no hooks.
- Input passthrough via `SetWindowLong` hit-test flags.

### Linux
- X11: ARGB visual + composite manager; uses XFixes region for input passthrough.
- Wayland: fallback to PipeWire stream-based surface where allowed (no game hooks).

### macOS
- NSWindow with `NSBorderlessWindowMask`, `setIgnoresMouseEvents` for passthrough.
- CoreAnimation layer tree for efficient composition; Metal layer optional.

## C API Sketch

```c
// dakt_overlay.h

typedef struct DaktOverlayManager* DaktOverlayManagerHandle;
typedef struct DaktOverlayWindow* DaktOverlayWindowHandle;

typedef struct {
    int x, y, width, height;
    float opacity;
    int clickThrough; // bool
} DaktOverlayConfig;

DAKT_API DaktOverlayManagerHandle dakt_overlay_create(void);
DAKT_API void dakt_overlay_destroy(DaktOverlayManagerHandle mgr);

DAKT_API DaktOverlayWindowHandle dakt_overlay_create_window(
    DaktOverlayManagerHandle mgr, const DaktOverlayConfig* cfg);
DAKT_API void dakt_overlay_destroy_window(DaktOverlayWindowHandle wnd);

DAKT_API void dakt_overlay_show(DaktOverlayWindowHandle wnd);
DAKT_API void dakt_overlay_hide(DaktOverlayWindowHandle wnd);
DAKT_API void dakt_overlay_set_bounds(DaktOverlayWindowHandle wnd, int x, int y, int w, int h);
DAKT_API void dakt_overlay_set_opacity(DaktOverlayWindowHandle wnd, float opacity);
DAKT_API void dakt_overlay_set_click_through(DaktOverlayWindowHandle wnd, int enable);
```

## Thread Safety

| Component | Safety |
|-----------|--------|
| OverlayManager | Main thread only |
| OverlayWindow | Main thread only |
| Painter/DrawList | Frame-local |
| Surfaces | Platform-thread affinity |

## Performance Considerations

1. **Batching**: Combine primitives to minimize uploads.
2. **Swapchain reuse**: Avoid recreating surfaces on resize where APIs allow.
3. **Premultiplied alpha**: Use throughout to avoid color fringes.
4. **Passthrough mode**: Disable input hit-testing when not needed.
5. **Minimal overdraw**: Clip layers; cull fully transparent items.
