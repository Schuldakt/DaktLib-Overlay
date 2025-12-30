# Overlay Module TODO

## Status: ⬜ Not Started

---

## Implementation Checklist

### Core Overlay
- [ ] `OverlayWindow.cpp` - Transparent window
- [ ] `HotkeyManager.cpp` - Global hotkeys
- [ ] `ProcessDetector.cpp` - Game detection
- [ ] `WindowTracker.cpp` - Position tracking

### Screen Capture
- [ ] `ScreenCapture.hpp` - Capture interface
- [ ] `BitBltCapture.cpp` - GDI capture
- [ ] `DXGICapture.cpp` - Desktop duplication
- [ ] `WGCCapture.cpp` - Windows Graphics Capture

### Panels
- [ ] `IPanel.hpp` - Panel interface
- [ ] `InfoPanel.cpp` - Basic info overlay
- [ ] `MapPanel.cpp` - Star map overlay
- [ ] `InventoryPanel.cpp` - Inventory helper

---

## Features

### Must Have
- [ ] Transparent overlay window
- [ ] Click-through mode
- [ ] Global hotkeys
- [ ] Game window tracking

### Should Have
- [ ] Screen capture
- [ ] Basic panels
- [ ] Minimize to tray

### Nice to Have
- [ ] Multiple monitor support
- [ ] Gamepad hotkeys
- [ ] Profile system

---

## Technical Challenges

- [ ] EasyAntiCheat compatibility testing
- [ ] Performance impact measurement
- [ ] HDR game support
- [ ] Borderless vs exclusive fullscreen

---

## Dependencies

- **Core** - Types, Platform
- **Logger** - Logging
- **GUI** - DaktGui rendering
- **Windows SDK** - Win32 APIs