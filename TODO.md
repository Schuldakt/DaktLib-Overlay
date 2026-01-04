# DaktLib-Overlay TODO

## Legend
- **Priority**: P0 (Critical) → P3 (Nice-to-have)
- **Complexity**: S (Small) | M (Medium) | L (Large) | XL (Extra Large)

---

## Phase 1: Foundation
- [ ] **[S]** CMake scaffolding, export macros
- [ ] **[S]** Core types (`OverlayManager`, `OverlayWindow`, `Layer`)
- [ ] **[S]** Math helpers (rects, color, transforms)
- [ ] **[M]** Error codes and logging hooks

## Phase 2: Layer & Draw Primitives
- [ ] **[M]** `Layer` with transform, opacity, clip rect
- [ ] **[M]** Primitives: rect, rounded rect, line, polyline, polygon, image, text
- [ ] **[S]** Layer tree traversal and z-ordering
- [ ] **[S]** Hit-test regions per layer

## Phase 3: Painter & Batching
- [ ] **[M]** `Painter` API to record draw commands
- [ ] **[M]** Batch by texture/state
- [ ] **[S]** Clip stack
- [ ] **[S]** Font atlas reuse (from DaktLib-GUI if available)

## Phase 4: Input Router
- [ ] **[M]** Hit-testing against layer bounds
- [ ] **[S]** Hover/click/scroll events
- [ ] **[S]** Click-through toggle (disable input)

## Phase 5: Surfaces (Cross-Platform Abstraction)
- [ ] **[M]** `ISurface` interface (resize, present, setOpacity, setHitTest)
- [ ] **[M]** SoftwareSurface fallback (CPU blit)
- [ ] **[S]** SwapchainSurface base for GPU-backed paths

## Phase 6: Windows Backend
- [ ] **[L]** WS_EX_LAYERED window creation
- [ ] **[M]** UpdateLayeredWindow path (per-pixel alpha)
- [ ] **[L]** DirectComposition path (Win10+) for better perf
- [ ] **[S]** Hit-test passthrough via region APIs

## Phase 7: Linux Backend
- [ ] **[L]** X11 ARGB window + composite manager detection
- [ ] **[M]** XFixes region for input passthrough
- [ ] **[L]** Wayland fallback via PipeWire stream where allowed

## Phase 8: macOS Backend
- [ ] **[L]** NSWindow setup (borderless, transparent)
- [ ] **[M]** CoreAnimation layer tree
- [ ] **[S]** Mouse ignore toggle

## Phase 9: Composition & Blending
- [ ] **[M]** Premultiplied alpha pipeline
- [ ] **[S]** Blend modes (normal, additive)
- [ ] **[S]** sRGB handling

## Phase 10: Window Management
- [ ] **[S]** Show/hide, move/resize APIs
- [ ] **[S]** Multi-monitor DPI awareness
- [ ] **[S]** Z-order management within overlays

## Phase 11: C API
- [ ] **[S]** Manager/window handles
- [ ] **[S]** Create/destroy window
- [ ] **[S]** Bounds/opacity/click-through setters
- [ ] **[S]** Present/draw entry point

## Phase 12: Testing & Samples
- [ ] **[M]** Unit tests for layer tree and hit-testing
- [ ] **[M]** Visual samples per platform
- [ ] **[S]** Latency and FPS measurements

## Milestones

| Milestone | Target | Phases |
|-----------|--------|--------|
| **v0.1.0** | Week 2 | 1-3 (core + painter) |
| **v0.2.0** | Week 4 | 4-5 (input + surfaces) |
| **v0.3.0** | Week 7 | 6 (Windows backend) |
| **v0.4.0** | Week 10 | 7 (Linux backend) |
| **v0.5.0** | Week 12 | 8-9 (macOS + blending) |
| **v0.6.0** | Week 13 | 10-11 (window mgmt + C API) |
| **v1.0.0** | Week 15 | 12 (tests/samples) |

## Estimated Effort
- Core + Windows path: ~6-7 weeks
- Linux + macOS paths: +5-6 weeks
- Polish/tests: +2 weeks
- Total: ~13-15 weeks

## Acceptance Criteria
- [ ] Overlay window appears above target app with per-pixel alpha
- [ ] No DLL injection or API hooks; works alongside EAC
- [ ] Click-through mode available and reliable
- [ ] Windows, Linux (X11/Wayland), macOS supported
- [ ] Basic primitives and text render correctly with premultiplied alpha
- [ ] C API usable from C# via ClangSharp
