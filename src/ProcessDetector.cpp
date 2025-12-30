// ============================================================================
// DaktLib - Overlay Module - Process Detector Implementation
// ============================================================================

#include <dakt/core/Time.hpp>
#include <dakt/logger/Logger.hpp>
#include <dakt/overlay/ProcessDetector.hpp>

#ifdef _WIN32
    #include <Psapi.h>
    #include <TlHelp32.h>
    #pragma comment(lib, "Psapi.lib")
#endif

#include <algorithm>

namespace dakt::overlay
{

// ============================================================================
// Constructor / Destructor
// ============================================================================

ProcessDetector::ProcessDetector() = default;

ProcessDetector::~ProcessDetector()
{
    shutdown();
}

ProcessDetector::ProcessDetector(ProcessDetector&& other) noexcept
    : m_initialized(other.m_initialized), m_monitoring(other.m_monitoring),
      m_targetProcessName(std::move(other.m_targetProcessName)), m_pollInterval(other.m_pollInterval),
      m_lastPollTime(other.m_lastPollTime), m_currentProcess(std::move(other.m_currentProcess)),
      m_startCallback(std::move(other.m_startCallback)), m_exitCallback(std::move(other.m_exitCallback))
{
    other.m_initialized = false;
    other.m_monitoring = false;
}

ProcessDetector& ProcessDetector::operator=(ProcessDetector&& other) noexcept
{
    if (this != &other)
    {
        shutdown();

        m_initialized = other.m_initialized;
        m_monitoring = other.m_monitoring;
        m_targetProcessName = std::move(other.m_targetProcessName);
        m_pollInterval = other.m_pollInterval;
        m_lastPollTime = other.m_lastPollTime;
        m_currentProcess = std::move(other.m_currentProcess);
        m_startCallback = std::move(other.m_startCallback);
        m_exitCallback = std::move(other.m_exitCallback);

        other.m_initialized = false;
        other.m_monitoring = false;
    }
    return *this;
}

// ============================================================================
// Initialization
// ============================================================================

bool ProcessDetector::initialize()
{
    if (m_initialized)
    {
        return true;
    }

    m_initialized = true;
    DAKT_LOG_DEBUG("Process detector initialized");
    return true;
}

void ProcessDetector::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    stopMonitoring();
    m_initialized = false;
    DAKT_LOG_DEBUG("Process detector shutdown");
}

// ============================================================================
// Process Detection
// ============================================================================

#ifdef _WIN32

struct EnumWindowsData
{
    u32 processId;
    HWND mainWindow;
    String windowTitle;
};

BOOL CALLBACK ProcessDetector::enumWindowsProc(HWND hwnd, LPARAM lParam)
{
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);

    DWORD windowProcessId = 0;
    GetWindowThreadProcessId(hwnd, &windowProcessId);

    if (windowProcessId == data->processId && IsWindowVisible(hwnd))
    {
        // Check if this is a main window (no owner)
        if (GetWindow(hwnd, GW_OWNER) == nullptr)
        {
            data->mainWindow = hwnd;

            // Get window title
            wchar_t title[256] = {};
            GetWindowTextW(hwnd, title, 256);

            char narrowTitle[512] = {};
            WideCharToMultiByte(CP_UTF8, 0, title, -1, narrowTitle, 512, nullptr, nullptr);
            data->windowTitle = narrowTitle;

            return FALSE;  // Stop enumeration
        }
    }

    return TRUE;
}

Option<ProcessInfo> ProcessDetector::findProcess(StringView processName)
{
    if (!m_initialized)
    {
        return std::nullopt;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return std::nullopt;
    }

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    // Convert process name to wide string for comparison
    std::wstring targetName(processName.begin(), processName.end());
    std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::towlower);

    Option<ProcessInfo> result;

    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            std::wstring exeName = entry.szExeFile;
            std::transform(exeName.begin(), exeName.end(), exeName.begin(), ::towlower);

            if (exeName == targetName)
            {
                ProcessInfo info;
                info.processId = entry.th32ProcessID;

                // Convert name back to narrow string
                char narrowName[512] = {};
                WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, narrowName, 512, nullptr, nullptr);
                info.processName = narrowName;

                // Find the main window
                info.windowHandle = getMainWindow(info.processId);

                // Check if elevated
                HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, info.processId);
                if (process)
                {
                    HANDLE token = nullptr;
                    if (OpenProcessToken(process, TOKEN_QUERY, &token))
                    {
                        TOKEN_ELEVATION elevation = {};
                        DWORD size = sizeof(elevation);
                        if (GetTokenInformation(token, TokenElevation, &elevation, size, &size))
                        {
                            info.isElevated = elevation.TokenIsElevated != 0;
                        }
                        CloseHandle(token);
                    }
                    CloseHandle(process);
                }

                result = info;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return result;
}

std::vector<ProcessInfo> ProcessDetector::findAllProcesses(StringView processName)
{
    std::vector<ProcessInfo> results;

    if (!m_initialized)
    {
        return results;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return results;
    }

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    std::wstring targetName(processName.begin(), processName.end());
    std::transform(targetName.begin(), targetName.end(), targetName.begin(), ::towlower);

    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            std::wstring exeName = entry.szExeFile;
            std::transform(exeName.begin(), exeName.end(), exeName.begin(), ::towlower);

            if (exeName == targetName)
            {
                ProcessInfo info;
                info.processId = entry.th32ProcessID;

                char narrowName[512] = {};
                WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, narrowName, 512, nullptr, nullptr);
                info.processName = narrowName;

                info.windowHandle = getMainWindow(info.processId);

                results.push_back(info);
            }
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return results;
}

Option<ProcessInfo> ProcessDetector::findWindowByClass(StringView className)
{
    if (!m_initialized)
    {
        return std::nullopt;
    }

    std::wstring wideClassName(className.begin(), className.end());
    HWND hwnd = FindWindowW(wideClassName.c_str(), nullptr);

    if (!hwnd)
    {
        return std::nullopt;
    }

    ProcessInfo info;
    info.windowHandle = hwnd;

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    info.processId = processId;

    // Get process name
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (process)
    {
        wchar_t exePath[MAX_PATH] = {};
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(process, 0, exePath, &size))
        {
            wchar_t* name = wcsrchr(exePath, L'\\');
            if (name)
            {
                char narrowName[512] = {};
                WideCharToMultiByte(CP_UTF8, 0, name + 1, -1, narrowName, 512, nullptr, nullptr);
                info.processName = narrowName;
            }
        }
        CloseHandle(process);
    }

    // Get window title
    wchar_t title[256] = {};
    GetWindowTextW(hwnd, title, 256);
    char narrowTitle[512] = {};
    WideCharToMultiByte(CP_UTF8, 0, title, -1, narrowTitle, 512, nullptr, nullptr);
    info.windowTitle = narrowTitle;

    return info;
}

Option<ProcessInfo> ProcessDetector::findWindowByTitle(StringView title)
{
    if (!m_initialized)
    {
        return std::nullopt;
    }

    std::wstring wideTitle(title.begin(), title.end());
    HWND hwnd = FindWindowW(nullptr, wideTitle.c_str());

    if (!hwnd)
    {
        return std::nullopt;
    }

    ProcessInfo info;
    info.windowHandle = hwnd;
    info.windowTitle = String(title);

    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    info.processId = processId;

    // Get process name
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (process)
    {
        wchar_t exePath[MAX_PATH] = {};
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(process, 0, exePath, &size))
        {
            wchar_t* name = wcsrchr(exePath, L'\\');
            if (name)
            {
                char narrowName[512] = {};
                WideCharToMultiByte(CP_UTF8, 0, name + 1, -1, narrowName, 512, nullptr, nullptr);
                info.processName = narrowName;
            }
        }
        CloseHandle(process);
    }

    return info;
}

bool ProcessDetector::isProcessRunning(StringView processName)
{
    return findProcess(processName).has_value();
}

void* ProcessDetector::getMainWindow(u32 processId)
{
    EnumWindowsData data = {};
    data.processId = processId;
    data.mainWindow = nullptr;

    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&data));

    return data.mainWindow;
}

std::vector<ProcessInfo> ProcessDetector::getAllProcesses()
{
    std::vector<ProcessInfo> results;

    if (!m_initialized)
    {
        return results;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return results;
    }

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            ProcessInfo info;
            info.processId = entry.th32ProcessID;

            char narrowName[512] = {};
            WideCharToMultiByte(CP_UTF8, 0, entry.szExeFile, -1, narrowName, 512, nullptr, nullptr);
            info.processName = narrowName;

            results.push_back(info);
        } while (Process32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return results;
}

bool ProcessDetector::isCurrentProcessElevated()
{
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
    {
        return false;
    }

    TOKEN_ELEVATION elevation = {};
    DWORD size = sizeof(elevation);
    bool elevated = false;

    if (GetTokenInformation(token, TokenElevation, &elevation, size, &size))
    {
        elevated = elevation.TokenIsElevated != 0;
    }

    CloseHandle(token);
    return elevated;
}

#else  // Non-Windows stubs

Option<ProcessInfo> ProcessDetector::findProcess(StringView)
{
    return std::nullopt;
}
std::vector<ProcessInfo> ProcessDetector::findAllProcesses(StringView)
{
    return {};
}
Option<ProcessInfo> ProcessDetector::findWindowByClass(StringView)
{
    return std::nullopt;
}
Option<ProcessInfo> ProcessDetector::findWindowByTitle(StringView)
{
    return std::nullopt;
}
bool ProcessDetector::isProcessRunning(StringView)
{
    return false;
}
void* ProcessDetector::getMainWindow(u32)
{
    return nullptr;
}
std::vector<ProcessInfo> ProcessDetector::getAllProcesses()
{
    return {};
}
bool ProcessDetector::isCurrentProcessElevated()
{
    return false;
}

#endif

// ============================================================================
// Monitoring
// ============================================================================

void ProcessDetector::startMonitoring(StringView processName, u32 pollInterval)
{
    m_targetProcessName = String(processName);
    m_pollInterval = pollInterval;
    m_monitoring = true;
    m_lastPollTime = 0;

    // Do an initial poll
    poll();

    DAKT_LOG_INFO("Started monitoring for process");
}

void ProcessDetector::stopMonitoring()
{
    m_monitoring = false;
    m_currentProcess = std::nullopt;
    DAKT_LOG_INFO("Stopped monitoring");
}

void ProcessDetector::poll()
{
    if (!m_monitoring)
    {
        return;
    }

    u64 currentTime = dakt::core::time::nowMillis();
    if (currentTime - m_lastPollTime < m_pollInterval)
    {
        return;
    }
    m_lastPollTime = currentTime;

    auto process = findProcess(m_targetProcessName);

    if (process && !m_currentProcess)
    {
        // Process started
        m_currentProcess = process;
        DAKT_LOG_INFO("Target process started");

        if (m_startCallback)
        {
            m_startCallback(*process);
        }
    }
    else if (!process && m_currentProcess)
    {
        // Process exited
        DAKT_LOG_INFO("Target process exited");

        if (m_exitCallback)
        {
            m_exitCallback(*m_currentProcess);
        }

        m_currentProcess = std::nullopt;
    }
}

}  // namespace dakt::overlay
