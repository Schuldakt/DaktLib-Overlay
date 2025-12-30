// ============================================================================
// DaktLib - Overlay Module - Panel System
// ============================================================================
// Overlay panel interface and management
// ============================================================================

#pragma once

#include <dakt/core/Geometry.hpp>
#include <dakt/core/String.hpp>
#include <dakt/core/Types.hpp>

#include <functional>
#include <memory>

namespace dakt::overlay
{

using namespace dakt::core;

// ============================================================================
// Panel Anchor
// ============================================================================

enum class PanelAnchor
{
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    Center,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    Custom  // Use explicit position
};

// ============================================================================
// Panel Flags
// ============================================================================

namespace PanelFlags
{
constexpr u32 None = 0;
constexpr u32 NoTitleBar = 1 << 0;
constexpr u32 NoMove = 1 << 1;
constexpr u32 NoResize = 1 << 2;
constexpr u32 NoClose = 1 << 3;
constexpr u32 AlwaysOnTop = 1 << 4;
constexpr u32 Transparent = 1 << 5;
constexpr u32 NoShadow = 1 << 6;
}  // namespace PanelFlags

// ============================================================================
// Panel Style
// ============================================================================

struct PanelStyle
{
    Color backgroundColor = {30, 30, 30, 200};
    Color borderColor = {60, 60, 60, 255};
    Color titleBarColor = {45, 45, 45, 255};
    Color titleTextColor = {220, 220, 220, 255};
    f32 borderWidth = 1.0f;
    f32 cornerRadius = 4.0f;
    f32 padding = 8.0f;
    f32 titleBarHeight = 24.0f;
};

// ============================================================================
// Panel Interface
// ============================================================================

class IPanel
{
public:
    virtual ~IPanel() = default;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /// Called when panel is added to overlay
    virtual void onAttach() {}

    /// Called when panel is removed from overlay
    virtual void onDetach() {}

    /// Called when panel becomes visible
    virtual void onShow() {}

    /// Called when panel is hidden
    virtual void onHide() {}

    // ========================================================================
    // Update and Render
    // ========================================================================

    /// Update panel logic
    /// @param deltaTime Time since last update in seconds
    virtual void update(f32 deltaTime) = 0;

    /// Render panel content
    /// @param bounds The content area to render into (excluding title bar)
    virtual void render(const Rect& bounds) = 0;

    // ========================================================================
    // Input
    // ========================================================================

    /// Handle mouse input
    /// @return true if input was consumed
    virtual bool onMouseDown(Vec2 pos, i32 button)
    {
        (void)pos;
        (void)button;
        return false;
    }
    virtual bool onMouseUp(Vec2 pos, i32 button)
    {
        (void)pos;
        (void)button;
        return false;
    }
    virtual bool onMouseMove(Vec2 pos)
    {
        (void)pos;
        return false;
    }
    virtual bool onMouseWheel(Vec2 pos, f32 delta)
    {
        (void)pos;
        (void)delta;
        return false;
    }

    /// Handle keyboard input
    /// @return true if input was consumed
    virtual bool onKeyDown(u32 virtualKey, u32 modifiers)
    {
        (void)virtualKey;
        (void)modifiers;
        return false;
    }
    virtual bool onKeyUp(u32 virtualKey, u32 modifiers)
    {
        (void)virtualKey;
        (void)modifiers;
        return false;
    }
    virtual bool onChar(char32_t character)
    {
        (void)character;
        return false;
    }

    // ========================================================================
    // Properties
    // ========================================================================

    /// Get panel name/ID
    [[nodiscard]] virtual StringView getName() const = 0;

    /// Get panel title (displayed in title bar)
    [[nodiscard]] virtual String getTitle() const { return String(getName()); }

    /// Get minimum size
    [[nodiscard]] virtual Vec2 getMinSize() const { return {100, 50}; }

    /// Get maximum size
    [[nodiscard]] virtual Vec2 getMaxSize() const { return {9999, 9999}; }

    /// Get default size
    [[nodiscard]] virtual Vec2 getDefaultSize() const { return {300, 200}; }

    /// Get anchor position
    [[nodiscard]] virtual PanelAnchor getAnchor() const { return PanelAnchor::TopLeft; }

    /// Get panel flags
    [[nodiscard]] virtual u32 getFlags() const { return PanelFlags::None; }

    /// Get panel style
    [[nodiscard]] virtual const PanelStyle& getStyle() const { return s_defaultStyle; }

protected:
    static PanelStyle s_defaultStyle;
};

// ============================================================================
// Panel Base Class
// ============================================================================

class PanelBase : public IPanel
{
public:
    explicit PanelBase(StringView name);
    ~PanelBase() override = default;

    [[nodiscard]] StringView getName() const override { return m_name; }
    [[nodiscard]] String getTitle() const override { return m_title.empty() ? String(m_name) : m_title; }

    void setTitle(StringView title) { m_title = String(title); }
    void setMinSize(Vec2 size) { m_minSize = size; }
    void setMaxSize(Vec2 size) { m_maxSize = size; }
    void setDefaultSize(Vec2 size) { m_defaultSize = size; }
    void setAnchor(PanelAnchor anchor) { m_anchor = anchor; }
    void setFlags(u32 flags) { m_flags = flags; }
    void setStyle(const PanelStyle& style) { m_style = style; }

    [[nodiscard]] Vec2 getMinSize() const override { return m_minSize; }
    [[nodiscard]] Vec2 getMaxSize() const override { return m_maxSize; }
    [[nodiscard]] Vec2 getDefaultSize() const override { return m_defaultSize; }
    [[nodiscard]] PanelAnchor getAnchor() const override { return m_anchor; }
    [[nodiscard]] u32 getFlags() const override { return m_flags; }
    [[nodiscard]] const PanelStyle& getStyle() const override { return m_style; }

protected:
    String m_name;
    String m_title;
    Vec2 m_minSize = {100, 50};
    Vec2 m_maxSize = {9999, 9999};
    Vec2 m_defaultSize = {300, 200};
    PanelAnchor m_anchor = PanelAnchor::TopLeft;
    u32 m_flags = PanelFlags::None;
    PanelStyle m_style;
};

// ============================================================================
// Simple Text Panel
// ============================================================================

class TextPanel : public PanelBase
{
public:
    explicit TextPanel(StringView name);
    ~TextPanel() override = default;

    void update(f32 deltaTime) override;
    void render(const Rect& bounds) override;

    void setText(StringView text) { m_text = String(text); }
    [[nodiscard]] const String& getText() const { return m_text; }

private:
    String m_text;
};

// ============================================================================
// Callback Panel
// ============================================================================

class CallbackPanel : public PanelBase
{
public:
    using UpdateFunc = std::function<void(f32)>;
    using RenderFunc = std::function<void(const Rect&)>;

    explicit CallbackPanel(StringView name);
    ~CallbackPanel() override = default;

    void update(f32 deltaTime) override;
    void render(const Rect& bounds) override;

    void setUpdateCallback(UpdateFunc callback) { m_updateCallback = std::move(callback); }
    void setRenderCallback(RenderFunc callback) { m_renderCallback = std::move(callback); }

private:
    UpdateFunc m_updateCallback;
    RenderFunc m_renderCallback;
};

}  // namespace dakt::overlay
