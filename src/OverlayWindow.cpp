// ============================================================================
// DaktLib - Overlay Module - Overlay Window Implementation
// ============================================================================

#include <dakt/logger/Logger.hpp>
#include <dakt/overlay/OverlayWindow.hpp>

#include <chrono>

#ifdef _WIN32
    #pragma comment(lib, "dwmapi.lib")
    #pragma comment(lib, "d3d11.lib")
    #pragma comment(lib, "dxgi.lib")

    #include <dwmapi.h>
#endif

namespace dakt::overlay
{

// ============================================================================
// Constructor / Destructor
// ============================================================================

OverlayWindow::OverlayWindow() = default;

OverlayWindow::~OverlayWindow()
{
    shutdown();
}

OverlayWindow::OverlayWindow(OverlayWindow&& other) noexcept
    : m_initialized(other.m_initialized), m_running(other.m_running), m_visible(other.m_visible),
      m_clickThrough(other.m_clickThrough), m_state(other.m_state), m_config(std::move(other.m_config)),
      m_hotkeyManager(std::move(other.m_hotkeyManager)), m_processDetector(std::move(other.m_processDetector)),
      m_windowTracker(std::move(other.m_windowTracker)), m_panels(std::move(other.m_panels))
#ifdef _WIN32
      ,
      m_hwnd(other.m_hwnd), m_hinstance(other.m_hinstance), m_device(other.m_device), m_context(other.m_context),
      m_swapChain(other.m_swapChain), m_renderTargetView(other.m_renderTargetView)
#endif
{
    other.m_initialized = false;
#ifdef _WIN32
    other.m_hwnd = nullptr;
    other.m_device = nullptr;
    other.m_context = nullptr;
    other.m_swapChain = nullptr;
    other.m_renderTargetView = nullptr;
#endif
}

OverlayWindow& OverlayWindow::operator=(OverlayWindow&& other) noexcept
{
    if (this != &other)
    {
        shutdown();

        m_initialized = other.m_initialized;
        m_running = other.m_running;
        m_visible = other.m_visible;
        m_clickThrough = other.m_clickThrough;
        m_state = other.m_state;
        m_config = std::move(other.m_config);
        m_hotkeyManager = std::move(other.m_hotkeyManager);
        m_processDetector = std::move(other.m_processDetector);
        m_windowTracker = std::move(other.m_windowTracker);
        m_panels = std::move(other.m_panels);

#ifdef _WIN32
        m_hwnd = other.m_hwnd;
        m_hinstance = other.m_hinstance;
        m_device = other.m_device;
        m_context = other.m_context;
        m_swapChain = other.m_swapChain;
        m_renderTargetView = other.m_renderTargetView;

        other.m_hwnd = nullptr;
        other.m_device = nullptr;
        other.m_context = nullptr;
        other.m_swapChain = nullptr;
        other.m_renderTargetView = nullptr;
#endif
        other.m_initialized = false;
    }
    return *this;
}

// ============================================================================
// Initialization
// ============================================================================

bool OverlayWindow::initialize(const OverlayConfig& config)
{
    if (m_initialized)
    {
        return true;
    }

    m_config = config;

    // Initialize components
    if (!m_hotkeyManager.initialize())
    {
        DAKT_LOG_ERROR("Failed to initialize hotkey manager");
        return false;
    }

    if (!m_processDetector.initialize())
    {
        DAKT_LOG_ERROR("Failed to initialize process detector");
        return false;
    }

    if (!m_windowTracker.initialize())
    {
        DAKT_LOG_ERROR("Failed to initialize window tracker");
        return false;
    }

    // Create the overlay window
    if (!createWindow())
    {
        DAKT_LOG_ERROR("Failed to create overlay window");
        return false;
    }

    // Initialize D3D11
    if (!initializeD3D11())
    {
        DAKT_LOG_ERROR("Failed to initialize D3D11");
        destroyWindow();
        return false;
    }

    // Set up window tracker callbacks
    m_windowTracker.setBoundsChangedCallback([this](const Rect&) { updateWindowPosition(); });

    m_windowTracker.setWindowClosedCallback([this]() { detach(); });

    m_initialized = true;
    m_state = OverlayState::Idle;
    m_visible = !config.startHidden;

    DAKT_LOG_INFO("Overlay window initialized");
    return true;
}

void OverlayWindow::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

    // Notify panels
    for (auto& entry : m_panels)
    {
        if (entry.panel)
        {
            entry.panel->onDetach();
        }
    }
    m_panels.clear();

    shutdownD3D11();
    destroyWindow();

    m_windowTracker.shutdown();
    m_processDetector.shutdown();
    m_hotkeyManager.shutdown();

    m_initialized = false;
    m_state = OverlayState::Uninitialized;

    DAKT_LOG_INFO("Overlay window shutdown");
}

// ============================================================================
// Process Attachment
// ============================================================================

bool OverlayWindow::attachToProcess(StringView processName)
{
    auto processInfo = m_processDetector.findProcess(processName);
    if (!processInfo)
    {
        DAKT_LOG_WARN("Process not found");
        return false;
    }

    return attachToWindow(processInfo->windowHandle);
}

bool OverlayWindow::attachToWindow(void* windowHandle)
{
    if (!windowHandle)
    {
        return false;
    }

#ifdef _WIN32
    if (!IsWindow(static_cast<HWND>(windowHandle)))
    {
        DAKT_LOG_ERROR("Invalid window handle");
        return false;
    }
#endif

    m_windowTracker.setTarget(windowHandle);
    m_windowTracker.refresh();

    if (!m_windowTracker.hasValidTarget())
    {
        DAKT_LOG_ERROR("Failed to attach to window");
        return false;
    }

    updateWindowPosition();

    m_state = OverlayState::Attached;
    if (m_stateChangeCallback)
    {
        m_stateChangeCallback(m_state);
    }

    DAKT_LOG_INFO("Attached to window");
    return true;
}

void OverlayWindow::detach()
{
    m_windowTracker.clearTarget();
    hide();

    m_state = OverlayState::Idle;
    if (m_stateChangeCallback)
    {
        m_stateChangeCallback(m_state);
    }
}

bool OverlayWindow::isAttached() const
{
    return m_windowTracker.hasValidTarget();
}

// ============================================================================
// Visibility Control
// ============================================================================

void OverlayWindow::show()
{
    if (!m_initialized || m_visible)
    {
        return;
    }

#ifdef _WIN32
    ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
#endif

    m_visible = true;

    if (isAttached())
    {
        m_state = OverlayState::Visible;
        if (m_stateChangeCallback)
        {
            m_stateChangeCallback(m_state);
        }
    }

    for (auto& entry : m_panels)
    {
        if (entry.visible && entry.panel)
        {
            entry.panel->onShow();
        }
    }

    if (m_visibilityCallback)
    {
        m_visibilityCallback(true);
    }
}

void OverlayWindow::hide()
{
    if (!m_initialized || !m_visible)
    {
        return;
    }

#ifdef _WIN32
    ShowWindow(m_hwnd, SW_HIDE);
#endif

    m_visible = false;

    if (isAttached())
    {
        m_state = OverlayState::Attached;
        if (m_stateChangeCallback)
        {
            m_stateChangeCallback(m_state);
        }
    }

    for (auto& entry : m_panels)
    {
        if (entry.visible && entry.panel)
        {
            entry.panel->onHide();
        }
    }

    if (m_visibilityCallback)
    {
        m_visibilityCallback(false);
    }
}

void OverlayWindow::toggle()
{
    if (m_visible)
    {
        hide();
    }
    else
    {
        show();
    }
}

void OverlayWindow::setClickThrough(bool enabled)
{
    if (m_clickThrough == enabled)
    {
        return;
    }

    m_clickThrough = enabled;

#ifdef _WIN32
    LONG_PTR exStyle = GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE);
    if (enabled)
    {
        exStyle |= WS_EX_TRANSPARENT;
    }
    else
    {
        exStyle &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, exStyle);
#endif
}

// ============================================================================
// Panel Management
// ============================================================================

void OverlayWindow::addPanel(StringView name, std::shared_ptr<IPanel> panel)
{
    if (!panel)
    {
        return;
    }

    // Check if panel already exists
    for (auto& entry : m_panels)
    {
        if (entry.name == name)
        {
            entry.panel->onDetach();
            entry.panel = panel;
            entry.panel->onAttach();
            return;
        }
    }

    PanelEntry entry;
    entry.name = String(name);
    entry.panel = panel;
    entry.visible = false;

    panel->onAttach();
    m_panels.push_back(std::move(entry));
}

void OverlayWindow::removePanel(StringView name)
{
    for (auto it = m_panels.begin(); it != m_panels.end(); ++it)
    {
        if (it->name == name)
        {
            if (it->panel)
            {
                it->panel->onDetach();
            }
            m_panels.erase(it);
            return;
        }
    }
}

std::shared_ptr<IPanel> OverlayWindow::getPanel(StringView name) const
{
    for (const auto& entry : m_panels)
    {
        if (entry.name == name)
        {
            return entry.panel;
        }
    }
    return nullptr;
}

void OverlayWindow::showPanel(StringView name)
{
    for (auto& entry : m_panels)
    {
        if (entry.name == name && !entry.visible)
        {
            entry.visible = true;
            if (m_visible && entry.panel)
            {
                entry.panel->onShow();
            }
            return;
        }
    }
}

void OverlayWindow::hidePanel(StringView name)
{
    for (auto& entry : m_panels)
    {
        if (entry.name == name && entry.visible)
        {
            entry.visible = false;
            if (m_visible && entry.panel)
            {
                entry.panel->onHide();
            }
            return;
        }
    }
}

void OverlayWindow::togglePanel(StringView name)
{
    for (auto& entry : m_panels)
    {
        if (entry.name == name)
        {
            if (entry.visible)
            {
                hidePanel(name);
            }
            else
            {
                showPanel(name);
            }
            return;
        }
    }
}

// ============================================================================
// Hotkey Management
// ============================================================================

bool OverlayWindow::addHotkey(u32 virtualKey, u32 modifiers, std::function<void()> callback)
{
    return m_hotkeyManager.registerHotkey(virtualKey, modifiers, std::move(callback));
}

bool OverlayWindow::addHotkey(u32 virtualKey, std::function<void()> callback)
{
    return m_hotkeyManager.registerHotkey(virtualKey, std::move(callback));
}

void OverlayWindow::removeHotkey(u32 virtualKey, u32 modifiers)
{
    m_hotkeyManager.unregisterHotkey(virtualKey, modifiers);
}

// ============================================================================
// Event Loop
// ============================================================================

void OverlayWindow::run()
{
    m_running = true;

    while (m_running)
    {
        if (!processMessages())
        {
            break;
        }

        update();

        // Simple frame timing
        std::this_thread::sleep_for(std::chrono::milliseconds(m_config.updateInterval));
    }

    m_running = false;
}

void OverlayWindow::stop()
{
    m_running = false;
}

bool OverlayWindow::processMessages()
{
#ifdef _WIN32
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
#endif
    return true;
}

void OverlayWindow::update()
{
    // Update window tracker
    m_windowTracker.update();

    // Update panels
    const f32 deltaTime = static_cast<f32>(m_config.updateInterval) / 1000.0f;
    for (auto& entry : m_panels)
    {
        if (entry.visible && entry.panel)
        {
            entry.panel->update(deltaTime);
        }
    }

    // Render if visible
    if (m_visible)
    {
        beginFrame();
        renderPanels();
        endFrame();
    }
}

// ============================================================================
// Rendering
// ============================================================================

void OverlayWindow::beginFrame()
{
#ifdef _WIN32
    if (!m_context || !m_renderTargetView)
    {
        return;
    }

    // Clear with transparent color
    f32 clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
    m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
#endif
}

void OverlayWindow::endFrame()
{
#ifdef _WIN32
    if (!m_swapChain)
    {
        return;
    }

    m_swapChain->Present(1, 0);
#endif
}

void* OverlayWindow::getDevice() const
{
#ifdef _WIN32
    return m_device;
#else
    return nullptr;
#endif
}

void* OverlayWindow::getDeviceContext() const
{
#ifdef _WIN32
    return m_context;
#else
    return nullptr;
#endif
}

// ============================================================================
// State
// ============================================================================

Rect OverlayWindow::getTargetBounds() const
{
    return m_windowTracker.getBounds();
}

Rect OverlayWindow::getOverlayBounds() const
{
#ifdef _WIN32
    RECT rect;
    if (GetWindowRect(m_hwnd, &rect))
    {
        return Rect{static_cast<f32>(rect.left), static_cast<f32>(rect.top),
                         static_cast<f32>(rect.right - rect.left), static_cast<f32>(rect.bottom - rect.top)};
    }
#endif
    return {};
}

// ============================================================================
// Callbacks
// ============================================================================

void OverlayWindow::setStateChangeCallback(StateChangeCallback callback)
{
    m_stateChangeCallback = std::move(callback);
}

void OverlayWindow::setVisibilityCallback(VisibilityCallback callback)
{
    m_visibilityCallback = std::move(callback);
}

// ============================================================================
// Platform-Specific Implementation
// ============================================================================

#ifdef _WIN32

bool OverlayWindow::createWindow()
{
    m_hinstance = GetModuleHandleW(nullptr);

    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = windowProc;
    wc.hInstance = m_hinstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"DaktOverlayClass";

    if (!RegisterClassExW(&wc))
    {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            return false;
        }
    }

    // Create layered window
    DWORD exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    if (m_config.clickThrough)
    {
        exStyle |= WS_EX_TRANSPARENT;
    }

    // Convert title to wide string
    std::wstring wideTitle(m_config.windowTitle.begin(), m_config.windowTitle.end());

    m_hwnd = CreateWindowExW(exStyle, L"DaktOverlayClass", wideTitle.c_str(), WS_POPUP, 0, 0, 800, 600, nullptr,
                             nullptr, m_hinstance, this);

    if (!m_hwnd)
    {
        return false;
    }

    // Set layered window attributes for per-pixel alpha
    SetLayeredWindowAttributes(m_hwnd, 0, 255, LWA_ALPHA);

    // Enable DWM composition for better transparency
    BOOL enabled = TRUE;
    DwmSetWindowAttribute(m_hwnd, DWMWA_NCRENDERING_ENABLED, &enabled, sizeof(enabled));

    // Make window transparent
    MARGINS margins = {-1};
    DwmExtendFrameIntoClientArea(m_hwnd, &margins);

    return true;
}

void OverlayWindow::destroyWindow()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

bool OverlayWindow::initializeD3D11()
{
    UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    #ifdef _DEBUG
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                                         D3D_FEATURE_LEVEL_10_0};

    D3D_FEATURE_LEVEL featureLevel;

    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createFlags, featureLevels,
                                   ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &m_device, &featureLevel, &m_context);

    if (FAILED(hr))
    {
        return false;
    }

    // Get DXGI factory
    IDXGIDevice* dxgiDevice = nullptr;
    hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    dxgiDevice->Release();
    if (FAILED(hr))
    {
        return false;
    }

    IDXGIFactory2* dxgiFactory = nullptr;
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));
    dxgiAdapter->Release();
    if (FAILED(hr))
    {
        return false;
    }

    // Create swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = 0;  // Auto-size
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    hr = dxgiFactory->CreateSwapChainForHwnd(m_device, m_hwnd, &swapChainDesc, nullptr, nullptr, &m_swapChain);
    dxgiFactory->Release();

    if (FAILED(hr))
    {
        // Fall back to older swap chain creation
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        IDXGIFactory2* factory2 = nullptr;
        hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory2));
        if (SUCCEEDED(hr))
        {
            hr = factory2->CreateSwapChainForHwnd(m_device, m_hwnd, &swapChainDesc, nullptr, nullptr, &m_swapChain);
            factory2->Release();
        }

        if (FAILED(hr))
        {
            return false;
        }
    }

    // Create render target view
    ID3D11Texture2D* backBuffer = nullptr;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr))
    {
        return false;
    }

    hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
    backBuffer->Release();

    return SUCCEEDED(hr);
}

void OverlayWindow::shutdownD3D11()
{
    if (m_renderTargetView)
    {
        m_renderTargetView->Release();
        m_renderTargetView = nullptr;
    }

    if (m_swapChain)
    {
        m_swapChain->Release();
        m_swapChain = nullptr;
    }

    if (m_context)
    {
        m_context->Release();
        m_context = nullptr;
    }

    if (m_device)
    {
        m_device->Release();
        m_device = nullptr;
    }
}

LRESULT CALLBACK OverlayWindow::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    OverlayWindow* self = nullptr;

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<OverlayWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    else
    {
        self = reinterpret_cast<OverlayWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self)
    {
        return self->handleMessage(msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT OverlayWindow::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_HOTKEY:
            (void)m_hotkeyManager.processHotkey(static_cast<i32>(wParam));
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            // Recreate render target on resize
            if (m_swapChain && m_renderTargetView)
            {
                m_context->OMSetRenderTargets(0, nullptr, nullptr);
                m_renderTargetView->Release();
                m_renderTargetView = nullptr;

                HRESULT hr = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
                if (SUCCEEDED(hr))
                {
                    ID3D11Texture2D* backBuffer = nullptr;
                    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
                    if (SUCCEEDED(hr))
                    {
                        m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
                        backBuffer->Release();
                    }
                }
            }
            return 0;

        default:
            return DefWindowProcW(m_hwnd, msg, wParam, lParam);
    }
}

#else  // Non-Windows stub implementations

bool OverlayWindow::createWindow()
{
    return false;
}
void OverlayWindow::destroyWindow() {}
bool OverlayWindow::initializeD3D11()
{
    return false;
}
void OverlayWindow::shutdownD3D11() {}

#endif

// ============================================================================
// Private Helpers
// ============================================================================

void OverlayWindow::updateWindowPosition()
{
#ifdef _WIN32
    if (!m_hwnd || !m_windowTracker.hasValidTarget())
    {
        return;
    }

    Rect bounds = m_windowTracker.getBounds();

    SetWindowPos(m_hwnd, HWND_TOPMOST, static_cast<int>(bounds.x()), static_cast<int>(bounds.y()),
                 static_cast<int>(bounds.width()), static_cast<int>(bounds.height()), SWP_NOACTIVATE);
#endif
}

void OverlayWindow::renderPanels()
{
    Rect overlayBounds = getOverlayBounds();

    for (auto& entry : m_panels)
    {
        if (!entry.visible || !entry.panel)
        {
            continue;
        }

        // Calculate panel bounds based on anchor
        Vec2 size = entry.panel->getDefaultSize();
        Vec2 pos{0, 0};

        switch (entry.panel->getAnchor())
        {
            case PanelAnchor::TopLeft:
                pos = Vec2{0, 0};
                break;
            case PanelAnchor::TopCenter:
                pos = Vec2{(overlayBounds.width() - size.x) / 2, 0};
                break;
            case PanelAnchor::TopRight:
                pos = Vec2{overlayBounds.width() - size.x, 0};
                break;
            case PanelAnchor::MiddleLeft:
                pos = Vec2{0, (overlayBounds.height() - size.y) / 2};
                break;
            case PanelAnchor::Center:
                pos = Vec2{(overlayBounds.width() - size.x) / 2, (overlayBounds.height() - size.y) / 2};
                break;
            case PanelAnchor::MiddleRight:
                pos = Vec2{overlayBounds.width() - size.x, (overlayBounds.height() - size.y) / 2};
                break;
            case PanelAnchor::BottomLeft:
                pos = Vec2{0, overlayBounds.height() - size.y};
                break;
            case PanelAnchor::BottomCenter:
                pos = Vec2{(overlayBounds.width() - size.x) / 2, overlayBounds.height() - size.y};
                break;
            case PanelAnchor::BottomRight:
                pos = Vec2{overlayBounds.width() - size.x, overlayBounds.height() - size.y};
                break;
            case PanelAnchor::Custom:
                break;
        }

        Rect panelBounds{pos.x, pos.y, size.x, size.y};
        entry.panel->render(panelBounds);
    }
}

}  // namespace dakt::overlay
