// ============================================================================
// DaktLib - Overlay Module - Hotkey Manager
// ============================================================================
// Global hotkey registration and handling
// ============================================================================

#pragma once

#include <dakt/core/String.hpp>
#include <dakt/core/Types.hpp>

#include <functional>
#include <unordered_map>

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
// Hotkey Modifiers
// ============================================================================

namespace HotkeyMod
{
constexpr u32 None = 0;
constexpr u32 Alt = 0x0001;      // MOD_ALT
constexpr u32 Control = 0x0002;  // MOD_CONTROL
constexpr u32 Shift = 0x0004;    // MOD_SHIFT
constexpr u32 Win = 0x0008;      // MOD_WIN
}  // namespace HotkeyMod

// ============================================================================
// Hotkey Info
// ============================================================================

struct HotkeyInfo
{
    u32 virtualKey = 0;
    u32 modifiers = 0;
    i32 id = 0;
    std::function<void()> callback;
    String description;
};

// ============================================================================
// Hotkey Manager
// ============================================================================

class HotkeyManager
{
public:
    HotkeyManager();
    ~HotkeyManager();

    // Non-copyable
    HotkeyManager(const HotkeyManager&) = delete;
    HotkeyManager& operator=(const HotkeyManager&) = delete;

    // Move semantics
    HotkeyManager(HotkeyManager&& other) noexcept;
    HotkeyManager& operator=(HotkeyManager&& other) noexcept;

    // ========================================================================
    // Initialization
    // ========================================================================

    /// Initialize the hotkey manager
    [[nodiscard]] bool initialize();

    /// Shutdown and unregister all hotkeys
    void shutdown();

    /// Check if initialized
    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    // ========================================================================
    // Hotkey Registration
    // ========================================================================

    /// Register a global hotkey
    /// @param virtualKey VK_* key code
    /// @param modifiers Combination of HotkeyMod flags
    /// @param callback Function to call when hotkey is pressed
    /// @param description Optional description for the hotkey
    /// @return true if registration succeeded
    [[nodiscard]] bool registerHotkey(u32 virtualKey, u32 modifiers, std::function<void()> callback,
                                      StringView description = "");

    /// Register a hotkey without modifiers
    [[nodiscard]] bool registerHotkey(u32 virtualKey, std::function<void()> callback, StringView description = "");

    /// Unregister a hotkey
    void unregisterHotkey(u32 virtualKey, u32 modifiers = 0);

    /// Unregister all hotkeys
    void unregisterAll();

    /// Check if a hotkey is registered
    [[nodiscard]] bool isRegistered(u32 virtualKey, u32 modifiers = 0) const;

    // ========================================================================
    // Event Processing
    // ========================================================================

    /// Process a hotkey message (call from window procedure)
    /// @param hotkeyId The hotkey ID from WM_HOTKEY
    /// @return true if the hotkey was handled
    [[nodiscard]] bool processHotkey(i32 hotkeyId);

    /// Process all pending hotkey messages (non-blocking)
    void processMessages();

    // ========================================================================
    // Information
    // ========================================================================

    /// Get all registered hotkeys
    [[nodiscard]] const std::unordered_map<u64, HotkeyInfo>& getHotkeys() const { return m_hotkeys; }

    /// Get hotkey count
    [[nodiscard]] usize getHotkeyCount() const { return m_hotkeys.size(); }

    /// Convert a virtual key to string
    [[nodiscard]] static String virtualKeyToString(u32 virtualKey);

    /// Convert modifiers to string
    [[nodiscard]] static String modifiersToString(u32 modifiers);

    /// Format a hotkey as a string (e.g., "Ctrl+Shift+F1")
    [[nodiscard]] static String formatHotkey(u32 virtualKey, u32 modifiers);

private:
    /// Generate a unique key from virtual key and modifiers
    [[nodiscard]] static u64 makeKey(u32 virtualKey, u32 modifiers);

    /// Generate a unique hotkey ID
    [[nodiscard]] i32 generateId();

private:
    bool m_initialized = false;
    i32 m_nextId = 1;

#ifdef _WIN32
    HWND m_hwnd = nullptr;  // Window handle for hotkey registration
#endif

    // Maps composite key (vk + mods) -> hotkey info
    std::unordered_map<u64, HotkeyInfo> m_hotkeys;

    // Maps hotkey ID -> composite key
    std::unordered_map<i32, u64> m_idToKey;
};

}  // namespace dakt::overlay
