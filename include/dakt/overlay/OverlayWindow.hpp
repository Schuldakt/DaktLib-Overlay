// ============================================================================
// DaktLib - Overlay Module - Overlay Window
// ============================================================================
// Transparent layered window for overlay rendering
// ============================================================================

#pragma once

#include <dakt/core/Geometry.hpp>
#include <dakt/core/String.hpp>
#include <dakt/core/Types.hpp>
#include <dakt/overlay/HotkeyManager.hpp>
#include <dakt/overlay/Panel.hpp>
#include <dakt/overlay/ProcessDetector.hpp>
#include <dakt/overlay/WindowTracker.hpp>

#include <functional>
#include <memory>
#include <vector>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
    #include <d3d11.h>
    #include <dxgi1_2.h>
#endif

namespace dakt::overlay
{

using namespace dakt::core;

// ============================================================================
// Overlay Configuration
// ============================================================================

struct OverlayConfig
{
    String windowTitle = "DaktOverlay";
    String targetProcess = "StarCitizen.exe";
    bool clickThrough = true;
    bool alwaysOnTop = true;
    bool startHidden = true;
    u32 updateInterval = 16;  // ms (~60 FPS)
};

// ============================================================================
// Overlay State
// ============================================================================

enum class OverlayState
{
    Uninitialized,
    Idle,      // Waiting for game
    Attached,  // Tracking game window
    Visible,   // Overlay shown
    Paused     // Temporarily paused
};

// ============================================================================
// Overlay Window
// ============================================================================

class OverlayWindow
{
public:
    OverlayWindow();
    ~OverlayWindow();

    // Non-copyable
    OverlayWindow(const OverlayWindow&) = delete;
    OverlayWindow& operator=(const OverlayWindow&) = delete;

    // Move semantics
    OverlayWindow(OverlayWindow&& other) noexcept;
    OverlayWindow& operator=(OverlayWindow&& other) noexcept;

    // ========================================================================
    // Initialization
    // ========================================================================

    /// Initialize the overlay window
    [[nodiscard]] bool initialize(const OverlayConfig& config = {});

    /// Shutdown and cleanup
    void shutdown();

    /// Check if initialized
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    // ========================================================================
    // Process Attachment
    // ========================================================================

    /// Attach to a specific process by name
    [[nodiscard]] bool attachToProcess(StringView processName);

    /// Attach to a specific window handle
    [[nodiscard]] bool attachToWindow(void* windowHandle);

    /// Detach from current target
    void detach();

    /// Check if attached
    [[nodiscard]] bool isAttached() const;

    // ========================================================================
    // Visibility Control
    // ========================================================================

    /// Show the overlay
    void show();

    /// Hide the overlay
    void hide();

    /// Toggle visibility
    void toggle();

    /// Check if visible
    [[nodiscard]] bool isVisible() const { return m_visible; }

    /// Set click-through mode
    void setClickThrough(bool enabled);

    /// Check if click-through
    [[nodiscard]] bool isClickThrough() const { return m_clickThrough; }

    // ========================================================================
    // Panel Management
    // ========================================================================

    /// Add a panel
    void addPanel(StringView name, std::shared_ptr<IPanel> panel);

    /// Remove a panel
    void removePanel(StringView name);

    /// Get a panel by name
    [[nodiscard]] std::shared_ptr<IPanel> getPanel(StringView name) const;

    /// Show a panel
    void showPanel(StringView name);

    /// Hide a panel
    void hidePanel(StringView name);

    /// Toggle a panel
    void togglePanel(StringView name);

    // ========================================================================
    // Hotkey Management
    // ========================================================================

    /// Add a global hotkey
    [[nodiscard]] bool addHotkey(u32 virtualKey, u32 modifiers, std::function<void()> callback);

    /// Add a global hotkey (simple version)
    [[nodiscard]] bool addHotkey(u32 virtualKey, std::function<void()> callback);

    /// Remove a hotkey
    void removeHotkey(u32 virtualKey, u32 modifiers = 0);

    /// Get the hotkey manager
    [[nodiscard]] HotkeyManager& getHotkeyManager() { return m_hotkeyManager; }

    // ========================================================================
    // Event Loop
    // ========================================================================

    /// Run the overlay message loop (blocking)
    void run();

    /// Request stop
    void stop();

    /// Process a single message iteration (non-blocking)
    [[nodiscard]] bool processMessages();

    /// Update the overlay (call from external loop)
    void update();

    // ========================================================================
    // Rendering
    // ========================================================================

    /// Begin a frame
    void beginFrame();

    /// End a frame and present
    void endFrame();

    /// Get the D3D11 device
    [[nodiscard]] void* getDevice() const;

    /// Get the D3D11 device context
    [[nodiscard]] void* getDeviceContext() const;

    // ========================================================================
    // State
    // ========================================================================

    /// Get current state
    [[nodiscard]] OverlayState getState() const { return m_state; }

    /// Get target window bounds
    [[nodiscard]] Rect getTargetBounds() const;

    /// Get overlay window bounds
    [[nodiscard]] Rect getOverlayBounds() const;

    // ========================================================================
    // Callbacks
    // ========================================================================

    using StateChangeCallback = std::function<void(OverlayState)>;
    using VisibilityCallback = std::function<void(bool)>;

    void setStateChangeCallback(StateChangeCallback callback);
    void setVisibilityCallback(VisibilityCallback callback);

private:
    // Platform-specific window creation
    bool createWindow();
    void destroyWindow();

    // D3D11 initialization
    bool initializeD3D11();
    void shutdownD3D11();

    // Window procedure
#ifdef _WIN32
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);
#endif

    // Update helpers
    void updateWindowPosition();
    void renderPanels();

private:
    bool m_initialized = false;
    bool m_running = false;
    bool m_visible = false;
    bool m_clickThrough = true;
    OverlayState m_state = OverlayState::Uninitialized;

    OverlayConfig m_config;

    // Components
    HotkeyManager m_hotkeyManager;
    ProcessDetector m_processDetector;
    WindowTracker m_windowTracker;

    // Panels
    struct PanelEntry
    {
        String name;
        std::shared_ptr<IPanel> panel;
        bool visible = false;
    };
    std::vector<PanelEntry> m_panels;

    // Callbacks
    StateChangeCallback m_stateChangeCallback;
    VisibilityCallback m_visibilityCallback;

#ifdef _WIN32
    HWND m_hwnd = nullptr;
    HINSTANCE m_hinstance = nullptr;

    // D3D11
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGISwapChain1* m_swapChain = nullptr;
    ID3D11RenderTargetView* m_renderTargetView = nullptr;
#endif
};

}  // namespace dakt::overlay
