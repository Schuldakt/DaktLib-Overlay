// ============================================================================
// DaktLib - Overlay Module - Panel Implementation
// ============================================================================

#include <dakt/overlay/Panel.hpp>

namespace dakt::overlay
{

// ============================================================================
// Static Members
// ============================================================================

PanelStyle IPanel::s_defaultStyle;

// ============================================================================
// PanelBase Implementation
// ============================================================================

PanelBase::PanelBase(StringView name) : m_name(name) {}

// ============================================================================
// TextPanel Implementation
// ============================================================================

TextPanel::TextPanel(StringView name) : PanelBase(name) {}

void TextPanel::update(f32 /*deltaTime*/)
{
    // Nothing to update for simple text panel
}

void TextPanel::render(const Rect& /*bounds*/)
{
    // Rendering would be done through the GUI module
    // This is a placeholder - actual implementation would use DrawList

    // Example (pseudo-code):
    // const PanelStyle& style = getStyle();
    // drawList.addRectFilled(bounds, style.backgroundColor, style.cornerRadius);
    // drawList.addRect(bounds, style.borderColor, style.borderWidth, style.cornerRadius);
    // drawList.addText(bounds, m_text, style.titleTextColor);
}

// ============================================================================
// CallbackPanel Implementation
// ============================================================================

CallbackPanel::CallbackPanel(StringView name) : PanelBase(name) {}

void CallbackPanel::update(f32 deltaTime)
{
    if (m_updateCallback)
    {
        m_updateCallback(deltaTime);
    }
}

void CallbackPanel::render(const Rect& bounds)
{
    if (m_renderCallback)
    {
        m_renderCallback(bounds);
    }
}

}  // namespace dakt::overlay
