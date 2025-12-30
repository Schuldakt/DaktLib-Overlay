# Overlay Module Architecture

## Overview

External overlay system for displaying information over the Star Citizen game window without DLL injection (EasyAntiCheat safe).

**Dependencies:** Core, Logger, GUI

**Platform:** Windows only

## Components

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Overlay Module                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    OverlayWindow                            │    │
│  │  - Transparent layered window                               │    │
│  │  - Click-through when inactive                              │    │
│  │  - Always on top                                            │    │
│  │  - Position synced to game window                           │    │
│  └─────────────────────────────────────────────────────────────┘    │
│                              │                                      │
│         ┌────────────────────┼────────────────────┐                 │
│         ▼                    ▼                    ▼                 │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐            │
│  │   Hotkey    │     │   Process   │     │   Screen    │            │
│  │  Manager    │     │  Detector   │     │  Capture    │            │
│  └─────────────┘     └─────────────┘     └─────────────┘            │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │                    Overlay Panels                           │    │
│  │  - Info panel        - Map overlay                          │    │
│  │  - Inventory helper  - Settings                             │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
```

## Key Features

- **Transparent Window** - Layered window with per-pixel alpha
- **Click-Through** - Input passes through when overlay inactive
- **Hotkey Activation** - Global hotkeys toggle overlay visibility
- **Process Detection** - Auto-detect when Star Citizen is running
- **Window Tracking** - Follow game window position/size changes
- **Screen Capture** - Capture game screen for OCR processing

## Screen Capture Methods

| Method | Performance | Compatibility |
|--------|-------------|---------------|
| BitBlt | Slow | Universal |
| DXGI Desktop Duplication | Fast | Win8+ |
| Windows Graphics Capture | Fast | Win10 1903+ |

## API Preview

```cpp
OverlayWindow overlay;
overlay.attachToProcess("StarCitizen.exe");

overlay.addHotkey(VK_F1, []() {
    overlay.togglePanel("info");
});

overlay.addPanel("info", makeShared<InfoPanel>());
overlay.addPanel("map", makeShared<MapPanel>());

overlay.run(); // Message loop
```

## Implementation Notes

- Uses Win32 layered windows (WS_EX_LAYERED)
- D3D11 rendering on transparent surface
- RegisterHotKey for global hotkeys
- FindWindow/EnumWindows for process detection
- SetWindowPos for window tracking