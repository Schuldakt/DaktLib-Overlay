// ============================================================================
// DaktLib - Overlay Module - Window Tracker
// ============================================================================
// Track target window position and size
// ============================================================================

#pragma once

#include <dakt/core/Geometry.hpp>
#include <dakt/core/String.hpp>
#include <dakt/core/Types.hpp>

#include <functional>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <Windows.h>
#endif

namespace dakt::overlay
{

using namespace dakt::core;

// ============================================================================
// Window State
// ============================================================================

struct WindowState
{
    Rect bounds;
    bool minimized = false;
    bool maximized = false;
    bool focused = false;
    bool valid = false;
};

// ============================================================================
// Window Tracker
// ============================================================================

class WindowTracker
{
public:
    WindowTracker();
    ~WindowTracker();

    // Non-copyable
    WindowTracker(const WindowTracker&) = delete;
    WindowTracker& operator=(const WindowTracker&) = delete;

    // Move semantics
    WindowTracker(WindowTracker&& other) noexcept;
    WindowTracker& operator=(WindowTracker&& other) noexcept;

    // ========================================================================
    // Initialization
    // ========================================================================

    /// Initialize the window tracker
    [[nodiscard]] bool initialize();

    /// Shutdown
    void shutdown();

    /// Check if initialized
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    // ========================================================================
    // Target Window
    // ========================================================================

    /// Set the target window to track
    /// @param windowHandle The window handle to track
    void setTarget(void* windowHandle);

    /// Clear the target window
    void clearTarget();

    /// Get the target window handle
    [[nodiscard]] void* getTarget() const { return m_targetWindow; }

    /// Check if tracking a valid window
    [[nodiscard]] bool hasValidTarget() const;

    // ========================================================================
    // Window State
    // ========================================================================

    /// Get the current window state
    [[nodiscard]] const WindowState& getState() const { return m_currentState; }

    /// Get the window bounds
    [[nodiscard]] Rect getBounds() const { return m_currentState.bounds; }

    /// Check if the window is minimized
    [[nodiscard]] bool isMinimized() const { return m_currentState.minimized; }

    /// Check if the window is maximized
    [[nodiscard]] bool isMaximized() const { return m_currentState.maximized; }

    /// Check if the window is focused
    [[nodiscard]] bool isFocused() const { return m_currentState.focused; }

    /// Check if the window is in borderless fullscreen mode
    [[nodiscard]] bool isBorderlessFullscreen() const;

    /// Check if the window is in exclusive fullscreen mode
    [[nodiscard]] bool isExclusiveFullscreen() const;

    // ========================================================================
    // Update
    // ========================================================================

    /// Update the window state (call periodically)
    void update();

    /// Force immediate state refresh
    void refresh();

    // ========================================================================
    // Callbacks
    // ========================================================================

    using BoundsChangedCallback = std::function<void(const Rect& newBounds)>;
    using StateChangedCallback = std::function<void(const WindowState& newState)>;
    using WindowClosedCallback = std::function<void()>;

    /// Set callback for when window bounds change
    void setBoundsChangedCallback(BoundsChangedCallback callback) { m_boundsChangedCallback = callback; }

    /// Set callback for when window state changes
    void setStateChangedCallback(StateChangedCallback callback) { m_stateChangedCallback = callback; }

    /// Set callback for when window is closed
    void setWindowClosedCallback(WindowClosedCallback callback) { m_windowClosedCallback = callback; }

    // ========================================================================
    // Monitor Information
    // ========================================================================

    /// Get the monitor containing the target window
    [[nodiscard]] Rect getMonitorBounds() const;

    /// Get the work area (excluding taskbar) of the monitor
    [[nodiscard]] Rect getMonitorWorkArea() const;

    /// Get all monitor bounds
    [[nodiscard]] std::vector<Rect> getAllMonitorBounds() const;

private:
    void updateWindowState();
    void notifyBoundsChanged();
    void notifyStateChanged();
    void notifyWindowClosed();

private:
    bool m_initialized = false;

#ifdef _WIN32
    HWND m_targetWindow = nullptr;
#else
    void* m_targetWindow = nullptr;
#endif

    WindowState m_currentState;
    WindowState m_previousState;

    BoundsChangedCallback m_boundsChangedCallback;
    StateChangedCallback m_stateChangedCallback;
    WindowClosedCallback m_windowClosedCallback;
};

}  // namespace dakt::overlay
