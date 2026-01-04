# AGENT Brief — DaktLib-Overlay

## Mission
- EAC-safe overlay compositor using OS-level translucent windows (no hooks/injection).

## Constraints
- C++23, dependency-free; cross-platform (Win X11/Wayland macOS).
- No API detours or game-process access; click-through optional.
- C API ClangSharp-friendly.

## Scope Highlights
- Layer tree, painter/batching, surfaces per OS (WS_EX_LAYERED/DirectComposition, X11/Wayland, NSWindow/CoreAnimation).
- Premultiplied alpha pipeline; composition and z-order control.

## Limitations
- No GPU API hooking; relies on top-level windows only.
- No external window managers; stay within public OS APIs.
