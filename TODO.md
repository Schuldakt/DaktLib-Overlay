# Overlay Module TODO

## Status: ✅ Completed

---

## Implementation Checklist

### Core Overlay
- [x] `OverlayWindow.cpp` - Transparent window (WS_EX_LAYERED, D3D11, DwmExtendFrameIntoClientArea)
- [x] `HotkeyManager.cpp` - Global hotkeys (RegisterHotKey/UnregisterHotKey)
- [x] `ProcessDetector.cpp` - Game detection (CreateToolhelp32Snapshot, EnumWindows)
- [x] `WindowTracker.cpp` - Position tracking (GetWindowRect, multi-monitor)

### Screen Capture
- [x] `ScreenCapture.hpp` - Capture interface (CapturedImage, ImageFormat)
- [x] `BitBltCapture.cpp` - GDI capture implementation
- [x] `DXGICapture.cpp` - Desktop Duplication API capture
- [ ] `WGCCapture.cpp` - Windows Graphics Capture (deferred - needs WinRT)

### Panels
- [x] `Panel.hpp` - Panel interface (PanelAnchor, IPanel, PanelBase)
- [x] `Panel.cpp` - TextPanel, CallbackPanel implementations
- [ ] `InfoPanel.cpp` - Basic info overlay (deferred to app layer)
- [ ] `MapPanel.cpp` - Star map overlay (deferred to app layer)
- [ ] `InventoryPanel.cpp` - Inventory helper (deferred to app layer)

---

## Features

### Must Have
- [x] Transparent overlay window
- [x] Click-through mode (WS_EX_TRANSPARENT)
- [x] Global hotkeys
- [x] Game window tracking

### Should Have
- [x] Screen capture (BitBlt + DXGI Desktop Duplication)
- [x] Basic panels
- [ ] Minimize to tray (app-layer feature)

### Nice to Have
- [x] Multiple monitor support
- [ ] Gamepad hotkeys (future)
- [ ] Profile system (future)

---

## Technical Implementation Notes

- Uses WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT for overlay
- D3D11 swap chain with DXGI_ALPHA_MODE_PREMULTIPLIED for transparency
- DwmExtendFrameIntoClientArea(-1,-1,-1,-1) for glass effect
- RegisterHotKey for global hotkey capture
- CreateToolhelp32Snapshot + EnumWindows for process/window detection
- BitBlt for simple GDI capture, DXGI Desktop Duplication for high-performance capture
- Image format conversion utilities (BGRA8→RGBA8→Grayscale), bilinear scaling, cropping

---

## Dependencies

- **Core** - Types, Platform, Time
- **Logger** - Logging
- **GUI** - DaktGui rendering, Types
- **Windows SDK** - Win32 APIs, D3D11, DXGI