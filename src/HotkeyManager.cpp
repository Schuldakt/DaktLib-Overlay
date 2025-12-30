// ============================================================================
// DaktLib - Overlay Module - Hotkey Manager Implementation
// ============================================================================

#include <dakt/logger/Logger.hpp>
#include <dakt/overlay/HotkeyManager.hpp>

namespace dakt::overlay
{

// ============================================================================
// Constructor / Destructor
// ============================================================================

HotkeyManager::HotkeyManager() = default;

HotkeyManager::~HotkeyManager()
{
    shutdown();
}

HotkeyManager::HotkeyManager(HotkeyManager&& other) noexcept
    : m_initialized(other.m_initialized), m_nextId(other.m_nextId)
#ifdef _WIN32
      ,
      m_hwnd(other.m_hwnd)
#endif
      ,
      m_hotkeys(std::move(other.m_hotkeys)), m_idToKey(std::move(other.m_idToKey))
{
    other.m_initialized = false;
#ifdef _WIN32
    other.m_hwnd = nullptr;
#endif
}

HotkeyManager& HotkeyManager::operator=(HotkeyManager&& other) noexcept
{
    if (this != &other)
    {
        shutdown();

        m_initialized = other.m_initialized;
        m_nextId = other.m_nextId;
#ifdef _WIN32
        m_hwnd = other.m_hwnd;
        other.m_hwnd = nullptr;
#endif
        m_hotkeys = std::move(other.m_hotkeys);
        m_idToKey = std::move(other.m_idToKey);
        other.m_initialized = false;
    }
    return *this;
}

// ============================================================================
// Initialization
// ============================================================================

bool HotkeyManager::initialize()
{
    if (m_initialized)
    {
        return true;
    }

    m_initialized = true;
    DAKT_LOG_DEBUG("Hotkey manager initialized");
    return true;
}

void HotkeyManager::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    unregisterAll();
    m_initialized = false;
    DAKT_LOG_DEBUG("Hotkey manager shutdown");
}

// ============================================================================
// Hotkey Registration
// ============================================================================

bool HotkeyManager::registerHotkey(u32 virtualKey, u32 modifiers, std::function<void()> callback,
                                   StringView description)
{
    if (!m_initialized || !callback)
    {
        return false;
    }

    u64 key = makeKey(virtualKey, modifiers);

    // Check if already registered
    if (m_hotkeys.contains(key))
    {
        DAKT_LOG_WARN("Hotkey already registered");
        return false;
    }

#ifdef _WIN32
    i32 id = generateId();

    // Register with Windows
    if (!RegisterHotKey(nullptr, id, modifiers, virtualKey))
    {
        DWORD error = GetLastError();
        if (error == ERROR_HOTKEY_ALREADY_REGISTERED)
        {
            DAKT_LOG_ERROR("Hotkey already registered by another application");
        }
        else
        {
            DAKT_LOG_ERROR("Failed to register hotkey");
        }
        return false;
    }

    HotkeyInfo info;
    info.virtualKey = virtualKey;
    info.modifiers = modifiers;
    info.id = id;
    info.callback = std::move(callback);
    info.description = String(description);

    m_hotkeys[key] = std::move(info);
    m_idToKey[id] = key;

    DAKT_LOG_DEBUG("Registered hotkey");
    return true;
#else
    (void)description;
    return false;
#endif
}

bool HotkeyManager::registerHotkey(u32 virtualKey, std::function<void()> callback, StringView description)
{
    return registerHotkey(virtualKey, HotkeyMod::None, std::move(callback), description);
}

void HotkeyManager::unregisterHotkey(u32 virtualKey, u32 modifiers)
{
    u64 key = makeKey(virtualKey, modifiers);

    auto it = m_hotkeys.find(key);
    if (it == m_hotkeys.end())
    {
        return;
    }

#ifdef _WIN32
    UnregisterHotKey(nullptr, it->second.id);
    m_idToKey.erase(it->second.id);
#endif

    m_hotkeys.erase(it);
    DAKT_LOG_DEBUG("Unregistered hotkey");
}

void HotkeyManager::unregisterAll()
{
#ifdef _WIN32
    for (const auto& [key, info] : m_hotkeys)
    {
        UnregisterHotKey(nullptr, info.id);
    }
#endif

    m_hotkeys.clear();
    m_idToKey.clear();
    DAKT_LOG_DEBUG("Unregistered all hotkeys");
}

bool HotkeyManager::isRegistered(u32 virtualKey, u32 modifiers) const
{
    return m_hotkeys.contains(makeKey(virtualKey, modifiers));
}

// ============================================================================
// Event Processing
// ============================================================================

bool HotkeyManager::processHotkey(i32 hotkeyId)
{
    auto it = m_idToKey.find(hotkeyId);
    if (it == m_idToKey.end())
    {
        return false;
    }

    auto hotkeyIt = m_hotkeys.find(it->second);
    if (hotkeyIt == m_hotkeys.end())
    {
        return false;
    }

    if (hotkeyIt->second.callback)
    {
        hotkeyIt->second.callback();
        return true;
    }

    return false;
}

void HotkeyManager::processMessages()
{
#ifdef _WIN32
    MSG msg;
    while (PeekMessageW(&msg, nullptr, WM_HOTKEY, WM_HOTKEY, PM_REMOVE))
    {
        if (msg.message == WM_HOTKEY)
        {
            (void)processHotkey(static_cast<i32>(msg.wParam));
        }
    }
#endif
}

// ============================================================================
// Utility Functions
// ============================================================================

String HotkeyManager::virtualKeyToString(u32 virtualKey)
{
#ifdef _WIN32
    // Handle special keys
    switch (virtualKey)
    {
        case VK_F1:
            return "F1";
        case VK_F2:
            return "F2";
        case VK_F3:
            return "F3";
        case VK_F4:
            return "F4";
        case VK_F5:
            return "F5";
        case VK_F6:
            return "F6";
        case VK_F7:
            return "F7";
        case VK_F8:
            return "F8";
        case VK_F9:
            return "F9";
        case VK_F10:
            return "F10";
        case VK_F11:
            return "F11";
        case VK_F12:
            return "F12";
        case VK_ESCAPE:
            return "Escape";
        case VK_TAB:
            return "Tab";
        case VK_RETURN:
            return "Enter";
        case VK_SPACE:
            return "Space";
        case VK_BACK:
            return "Backspace";
        case VK_DELETE:
            return "Delete";
        case VK_INSERT:
            return "Insert";
        case VK_HOME:
            return "Home";
        case VK_END:
            return "End";
        case VK_PRIOR:
            return "PageUp";
        case VK_NEXT:
            return "PageDown";
        case VK_UP:
            return "Up";
        case VK_DOWN:
            return "Down";
        case VK_LEFT:
            return "Left";
        case VK_RIGHT:
            return "Right";
        case VK_NUMPAD0:
            return "Num0";
        case VK_NUMPAD1:
            return "Num1";
        case VK_NUMPAD2:
            return "Num2";
        case VK_NUMPAD3:
            return "Num3";
        case VK_NUMPAD4:
            return "Num4";
        case VK_NUMPAD5:
            return "Num5";
        case VK_NUMPAD6:
            return "Num6";
        case VK_NUMPAD7:
            return "Num7";
        case VK_NUMPAD8:
            return "Num8";
        case VK_NUMPAD9:
            return "Num9";
        case VK_MULTIPLY:
            return "Num*";
        case VK_ADD:
            return "Num+";
        case VK_SUBTRACT:
            return "Num-";
        case VK_DIVIDE:
            return "Num/";
        case VK_DECIMAL:
            return "Num.";
        default:
            break;
    }

    // Try to get the key name from Windows
    UINT scanCode = MapVirtualKeyW(virtualKey, MAPVK_VK_TO_VSC);
    wchar_t keyName[64] = {};

    if (GetKeyNameTextW(static_cast<LONG>(scanCode << 16), keyName, 64) > 0)
    {
        // Convert to narrow string
        char narrowName[64] = {};
        WideCharToMultiByte(CP_UTF8, 0, keyName, -1, narrowName, 64, nullptr, nullptr);
        return String(narrowName);
    }

    // Fall back to character
    if (virtualKey >= 'A' && virtualKey <= 'Z')
    {
        return String(1, static_cast<char>(virtualKey));
    }
    if (virtualKey >= '0' && virtualKey <= '9')
    {
        return String(1, static_cast<char>(virtualKey));
    }
#else
    (void)virtualKey;
#endif

    return "Unknown";
}

String HotkeyManager::modifiersToString(u32 modifiers)
{
    String result;

    if (modifiers & HotkeyMod::Control)
    {
        result += "Ctrl+";
    }
    if (modifiers & HotkeyMod::Alt)
    {
        result += "Alt+";
    }
    if (modifiers & HotkeyMod::Shift)
    {
        result += "Shift+";
    }
    if (modifiers & HotkeyMod::Win)
    {
        result += "Win+";
    }

    return result;
}

String HotkeyManager::formatHotkey(u32 virtualKey, u32 modifiers)
{
    return modifiersToString(modifiers) + virtualKeyToString(virtualKey);
}

u64 HotkeyManager::makeKey(u32 virtualKey, u32 modifiers)
{
    return (static_cast<u64>(modifiers) << 32) | virtualKey;
}

i32 HotkeyManager::generateId()
{
    return m_nextId++;
}

}  // namespace dakt::overlay
