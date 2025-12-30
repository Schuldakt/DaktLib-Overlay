// ============================================================================
// DaktLib - Overlay Module - Process Detector
// ============================================================================
// Detect and monitor target processes
// ============================================================================

#pragma once

#include <dakt/core/String.hpp>
#include <dakt/core/Types.hpp>

#include <functional>
#include <vector>

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
// Process Information
// ============================================================================

struct ProcessInfo
{
    u32 processId = 0;
    String processName;
    String windowTitle;
    void* windowHandle = nullptr;
    bool isElevated = false;
};

// ============================================================================
// Process Detector
// ============================================================================

class ProcessDetector
{
public:
    ProcessDetector();
    ~ProcessDetector();

    // Non-copyable
    ProcessDetector(const ProcessDetector&) = delete;
    ProcessDetector& operator=(const ProcessDetector&) = delete;

    // Move semantics
    ProcessDetector(ProcessDetector&& other) noexcept;
    ProcessDetector& operator=(ProcessDetector&& other) noexcept;

    // ========================================================================
    // Initialization
    // ========================================================================

    /// Initialize the process detector
    [[nodiscard]] bool initialize();

    /// Shutdown
    void shutdown();

    /// Check if initialized
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    // ========================================================================
    // Process Detection
    // ========================================================================

    /// Find a process by executable name
    /// @param processName Process name (e.g., "StarCitizen.exe")
    /// @return Process info if found
    [[nodiscard]] Option<ProcessInfo> findProcess(StringView processName);

    /// Find all instances of a process
    /// @param processName Process name to search for
    /// @return Vector of matching process infos
    [[nodiscard]] std::vector<ProcessInfo> findAllProcesses(StringView processName);

    /// Find a window by class name
    /// @param className Window class name
    /// @return Process info if found
    [[nodiscard]] Option<ProcessInfo> findWindowByClass(StringView className);

    /// Find a window by title
    /// @param title Window title (partial match)
    /// @return Process info if found
    [[nodiscard]] Option<ProcessInfo> findWindowByTitle(StringView title);

    /// Check if a process is running
    /// @param processName Process name to check
    /// @return true if at least one instance is running
    [[nodiscard]] bool isProcessRunning(StringView processName);

    /// Get the main window handle for a process
    /// @param processId Process ID
    /// @return Window handle or nullptr
    [[nodiscard]] void* getMainWindow(u32 processId);

    // ========================================================================
    // Monitoring
    // ========================================================================

    using ProcessCallback = std::function<void(const ProcessInfo&)>;

    /// Set callback for when target process starts
    void setProcessStartCallback(ProcessCallback callback) { m_startCallback = callback; }

    /// Set callback for when target process exits
    void setProcessExitCallback(ProcessCallback callback) { m_exitCallback = callback; }

    /// Start monitoring for a process
    /// @param processName Process name to monitor
    /// @param pollInterval Check interval in milliseconds
    void startMonitoring(StringView processName, u32 pollInterval = 1000);

    /// Stop monitoring
    void stopMonitoring();

    /// Check if currently monitoring
    [[nodiscard]] bool isMonitoring() const { return m_monitoring; }

    /// Poll for process changes (call periodically if not using callbacks)
    void poll();

    // ========================================================================
    // Information
    // ========================================================================

    /// Get the currently monitored process info
    [[nodiscard]] const Option<ProcessInfo>& getCurrentProcess() const { return m_currentProcess; }

    /// Get list of all running processes
    [[nodiscard]] std::vector<ProcessInfo> getAllProcesses();

    /// Check if the current process is elevated (admin)
    [[nodiscard]] static bool isCurrentProcessElevated();

private:
    bool m_initialized = false;
    bool m_monitoring = false;

    String m_targetProcessName;
    u32 m_pollInterval = 1000;
    u64 m_lastPollTime = 0;

    Option<ProcessInfo> m_currentProcess;

    ProcessCallback m_startCallback;
    ProcessCallback m_exitCallback;

#ifdef _WIN32
    // Internal helpers
    static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);
#endif
};

}  // namespace dakt::overlay
