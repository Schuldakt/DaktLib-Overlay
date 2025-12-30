// ============================================================================
// DaktLib - Overlay Module - Window Tracker Implementation
// ============================================================================

#include <dakt/logger/Logger.hpp>
#include <dakt/overlay/WindowTracker.hpp>

namespace dakt::overlay
{

// ============================================================================
// Constructor / Destructor
// ============================================================================

WindowTracker::WindowTracker() = default;

WindowTracker::~WindowTracker()
{
    shutdown();
}

WindowTracker::WindowTracker(WindowTracker&& other) noexcept
    : m_initialized(other.m_initialized)
#ifdef _WIN32
      ,
      m_targetWindow(other.m_targetWindow)
#endif
      ,
      m_currentState(other.m_currentState), m_previousState(other.m_previousState),
      m_boundsChangedCallback(std::move(other.m_boundsChangedCallback)),
      m_stateChangedCallback(std::move(other.m_stateChangedCallback)),
      m_windowClosedCallback(std::move(other.m_windowClosedCallback))
{
    other.m_initialized = false;
#ifdef _WIN32
    other.m_targetWindow = nullptr;
#endif
}

WindowTracker& WindowTracker::operator=(WindowTracker&& other) noexcept
{
    if (this != &other)
    {
        shutdown();

        m_initialized = other.m_initialized;
#ifdef _WIN32
        m_targetWindow = other.m_targetWindow;
        other.m_targetWindow = nullptr;
#endif
        m_currentState = other.m_currentState;
        m_previousState = other.m_previousState;
        m_boundsChangedCallback = std::move(other.m_boundsChangedCallback);
        m_stateChangedCallback = std::move(other.m_stateChangedCallback);
        m_windowClosedCallback = std::move(other.m_windowClosedCallback);

        other.m_initialized = false;
    }
    return *this;
}

// ============================================================================
// Initialization
// ============================================================================

bool WindowTracker::initialize()
{
    if (m_initialized)
    {
        return true;
    }

    m_initialized = true;
    DAKT_LOG_DEBUG("Window tracker initialized");
    return true;
}

void WindowTracker::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    clearTarget();
    m_initialized = false;
    DAKT_LOG_DEBUG("Window tracker shutdown");
}

// ============================================================================
// Target Window
// ============================================================================

void WindowTracker::setTarget(void* windowHandle)
{
#ifdef _WIN32
    m_targetWindow = static_cast<HWND>(windowHandle);
#else
    m_targetWindow = windowHandle;
#endif

    refresh();
    DAKT_LOG_DEBUG("Window tracker target set");
}

void WindowTracker::clearTarget()
{
#ifdef _WIN32
    m_targetWindow = nullptr;
#else
    m_targetWindow = nullptr;
#endif

    m_currentState = WindowState{};
    m_previousState = WindowState{};
}

bool WindowTracker::hasValidTarget() const
{
#ifdef _WIN32
    return m_targetWindow != nullptr && IsWindow(m_targetWindow);
#else
    return m_targetWindow != nullptr;
#endif
}

// ============================================================================
// Window State
// ============================================================================

bool WindowTracker::isBorderlessFullscreen() const
{
#ifdef _WIN32
    if (!hasValidTarget())
    {
        return false;
    }

    // Get window style
    LONG style = GetWindowLongW(m_targetWindow, GWL_STYLE);

    // Borderless windows typically have no WS_BORDER or WS_DLGFRAME
    bool noBorder = (style & WS_BORDER) == 0 && (style & WS_DLGFRAME) == 0;

    // Check if window covers the entire monitor
    gui::Rect monitorBounds = getMonitorBounds();

    return noBorder && m_currentState.bounds.x() <= monitorBounds.x() &&
           m_currentState.bounds.y() <= monitorBounds.y() && m_currentState.bounds.width() >= monitorBounds.width() &&
           m_currentState.bounds.height() >= monitorBounds.height();
#else
    return false;
#endif
}

bool WindowTracker::isExclusiveFullscreen() const
{
#ifdef _WIN32
    if (!hasValidTarget())
    {
        return false;
    }

    // In exclusive fullscreen, the window usually has no visible style
    // and may have different characteristics
    // This is a simplified check - real detection may need D3D/DXGI
    LONG style = GetWindowLongW(m_targetWindow, GWL_STYLE);
    return style == 0 || (style & WS_POPUP) != 0;
#else
    return false;
#endif
}

// ============================================================================
// Update
// ============================================================================

void WindowTracker::update()
{
    if (!hasValidTarget())
    {
        if (m_currentState.valid)
        {
            m_currentState.valid = false;
            notifyWindowClosed();
        }
        return;
    }

    m_previousState = m_currentState;
    updateWindowState();

    // Check for changes
    bool boundsChanged = m_currentState.bounds.x() != m_previousState.bounds.x() ||
                         m_currentState.bounds.y() != m_previousState.bounds.y() ||
                         m_currentState.bounds.width() != m_previousState.bounds.width() ||
                         m_currentState.bounds.height() != m_previousState.bounds.height();

    bool stateChanged = m_currentState.minimized != m_previousState.minimized ||
                        m_currentState.maximized != m_previousState.maximized ||
                        m_currentState.focused != m_previousState.focused;

    if (boundsChanged)
    {
        notifyBoundsChanged();
    }

    if (stateChanged)
    {
        notifyStateChanged();
    }
}

void WindowTracker::refresh()
{
    m_previousState = m_currentState;
    updateWindowState();
}

// ============================================================================
// Monitor Information
// ============================================================================

gui::Rect WindowTracker::getMonitorBounds() const
{
#ifdef _WIN32
    if (!hasValidTarget())
    {
        // Return primary monitor bounds
        return gui::Rect{0, 0, static_cast<f32>(GetSystemMetrics(SM_CXSCREEN)),
                         static_cast<f32>(GetSystemMetrics(SM_CYSCREEN))};
    }

    HMONITOR monitor = MonitorFromWindow(m_targetWindow, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info = {};
    info.cbSize = sizeof(info);

    if (GetMonitorInfoW(monitor, &info))
    {
        return gui::Rect{static_cast<f32>(info.rcMonitor.left), static_cast<f32>(info.rcMonitor.top),
                         static_cast<f32>(info.rcMonitor.right - info.rcMonitor.left),
                         static_cast<f32>(info.rcMonitor.bottom - info.rcMonitor.top)};
    }
#endif

    return gui::Rect{0, 0, 1920, 1080};
}

gui::Rect WindowTracker::getMonitorWorkArea() const
{
#ifdef _WIN32
    if (!hasValidTarget())
    {
        RECT workArea;
        if (SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0))
        {
            return gui::Rect{static_cast<f32>(workArea.left), static_cast<f32>(workArea.top),
                             static_cast<f32>(workArea.right - workArea.left),
                             static_cast<f32>(workArea.bottom - workArea.top)};
        }
    }
    else
    {
        HMONITOR monitor = MonitorFromWindow(m_targetWindow, MONITOR_DEFAULTTONEAREST);
        MONITORINFO info = {};
        info.cbSize = sizeof(info);

        if (GetMonitorInfoW(monitor, &info))
        {
            return gui::Rect{static_cast<f32>(info.rcWork.left), static_cast<f32>(info.rcWork.top),
                             static_cast<f32>(info.rcWork.right - info.rcWork.left),
                             static_cast<f32>(info.rcWork.bottom - info.rcWork.top)};
        }
    }
#endif

    return getMonitorBounds();
}

std::vector<gui::Rect> WindowTracker::getAllMonitorBounds() const
{
    std::vector<gui::Rect> results;

#ifdef _WIN32
    EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR monitor, HDC, LPRECT, LPARAM lParam) -> BOOL
        {
            auto* results = reinterpret_cast<std::vector<gui::Rect>*>(lParam);

            MONITORINFO info = {};
            info.cbSize = sizeof(info);

            if (GetMonitorInfoW(monitor, &info))
            {
                results->push_back(gui::Rect{static_cast<f32>(info.rcMonitor.left),
                                             static_cast<f32>(info.rcMonitor.top),
                                             static_cast<f32>(info.rcMonitor.right - info.rcMonitor.left),
                                             static_cast<f32>(info.rcMonitor.bottom - info.rcMonitor.top)});
            }

            return TRUE;
        },
        reinterpret_cast<LPARAM>(&results));
#endif

    if (results.empty())
    {
        results.push_back(getMonitorBounds());
    }

    return results;
}

// ============================================================================
// Private Implementation
// ============================================================================

void WindowTracker::updateWindowState()
{
#ifdef _WIN32
    if (!m_targetWindow || !IsWindow(m_targetWindow))
    {
        m_currentState.valid = false;
        return;
    }

    m_currentState.valid = true;

    // Get window rect
    RECT rect;
    if (GetWindowRect(m_targetWindow, &rect))
    {
        m_currentState.bounds =
            gui::Rect{static_cast<f32>(rect.left), static_cast<f32>(rect.top), static_cast<f32>(rect.right - rect.left),
                      static_cast<f32>(rect.bottom - rect.top)};
    }

    // Check window state
    WINDOWPLACEMENT placement = {};
    placement.length = sizeof(placement);
    if (GetWindowPlacement(m_targetWindow, &placement))
    {
        m_currentState.minimized = (placement.showCmd == SW_SHOWMINIMIZED);
        m_currentState.maximized = (placement.showCmd == SW_SHOWMAXIMIZED);
    }

    // Check focus
    m_currentState.focused = (GetForegroundWindow() == m_targetWindow);
#else
    m_currentState.valid = m_targetWindow != nullptr;
#endif
}

void WindowTracker::notifyBoundsChanged()
{
    if (m_boundsChangedCallback)
    {
        m_boundsChangedCallback(m_currentState.bounds);
    }
}

void WindowTracker::notifyStateChanged()
{
    if (m_stateChangedCallback)
    {
        m_stateChangedCallback(m_currentState);
    }
}

void WindowTracker::notifyWindowClosed()
{
    DAKT_LOG_INFO("Target window closed");

    if (m_windowClosedCallback)
    {
        m_windowClosedCallback();
    }
}

}  // namespace dakt::overlay
