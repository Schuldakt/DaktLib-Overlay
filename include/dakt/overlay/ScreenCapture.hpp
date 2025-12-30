// ============================================================================
// DaktLib - Overlay Module - Screen Capture
// ============================================================================
// Screen capture for OCR and other processing
// ============================================================================

#pragma once

#include <dakt/core/String.hpp>
#include <dakt/core/Types.hpp>
#include <dakt/gui/Types.hpp>

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
// Capture Format
// ============================================================================

enum class CaptureFormat
{
    BGRA8,     // 32-bit BGRA (Windows native)
    RGBA8,     // 32-bit RGBA
    RGB8,      // 24-bit RGB
    Grayscale  // 8-bit grayscale
};

// ============================================================================
// Capture Method
// ============================================================================

enum class CaptureMethod
{
    Auto,    // Choose best available method
    BitBlt,  // GDI BitBlt (slow, universal)
    DXGI,    // DXGI Desktop Duplication (fast, Win8+)
    WGC      // Windows Graphics Capture (fast, Win10 1903+)
};

// ============================================================================
// Captured Image
// ============================================================================

struct CapturedImage
{
    std::vector<byte> data;
    u32 width = 0;
    u32 height = 0;
    u32 stride = 0;  // Bytes per row
    CaptureFormat format = CaptureFormat::BGRA8;
    u64 timestamp = 0;  // Capture time in milliseconds
};

// ============================================================================
// Screen Capture Interface
// ============================================================================

class IScreenCapture
{
public:
    virtual ~IScreenCapture() = default;

    /// Initialize the capture system
    [[nodiscard]] virtual bool initialize() = 0;

    /// Shutdown and release resources
    virtual void shutdown() = 0;

    /// Check if initialized
    [[nodiscard]] virtual bool isInitialized() const = 0;

    /// Get the capture method
    [[nodiscard]] virtual CaptureMethod getMethod() const = 0;

    /// Capture the entire screen
    [[nodiscard]] virtual Option<CapturedImage> captureScreen() = 0;

    /// Capture a specific window
    [[nodiscard]] virtual Option<CapturedImage> captureWindow(void* windowHandle) = 0;

    /// Capture a region of the screen
    [[nodiscard]] virtual Option<CapturedImage> captureRegion(const gui::Rect& region) = 0;

    /// Set the output format
    virtual void setFormat(CaptureFormat format) = 0;

    /// Get the current output format
    [[nodiscard]] virtual CaptureFormat getFormat() const = 0;
};

// ============================================================================
// Screen Capture Factory
// ============================================================================

class ScreenCapture
{
public:
    /// Create a screen capture instance
    /// @param method The capture method to use (Auto selects best available)
    /// @return Screen capture instance or nullptr if creation fails
    [[nodiscard]] static std::unique_ptr<IScreenCapture> create(CaptureMethod method = CaptureMethod::Auto);

    /// Check if a capture method is available
    [[nodiscard]] static bool isMethodAvailable(CaptureMethod method);

    /// Get the best available capture method
    [[nodiscard]] static CaptureMethod getBestMethod();

    /// Convert an image to a different format
    [[nodiscard]] static Option<CapturedImage> convertFormat(const CapturedImage& image, CaptureFormat targetFormat);

    /// Scale an image
    [[nodiscard]] static Option<CapturedImage> scale(const CapturedImage& image, u32 newWidth, u32 newHeight);

    /// Crop an image
    [[nodiscard]] static Option<CapturedImage> crop(const CapturedImage& image, const gui::Rect& region);
};

// ============================================================================
// BitBlt Capture (GDI)
// ============================================================================

class BitBltCapture : public IScreenCapture
{
public:
    BitBltCapture();
    ~BitBltCapture() override;

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override { return m_initialized; }
    [[nodiscard]] CaptureMethod getMethod() const override { return CaptureMethod::BitBlt; }

    [[nodiscard]] Option<CapturedImage> captureScreen() override;
    [[nodiscard]] Option<CapturedImage> captureWindow(void* windowHandle) override;
    [[nodiscard]] Option<CapturedImage> captureRegion(const gui::Rect& region) override;

    void setFormat(CaptureFormat format) override { m_format = format; }
    [[nodiscard]] CaptureFormat getFormat() const override { return m_format; }

private:
    bool m_initialized = false;
    CaptureFormat m_format = CaptureFormat::BGRA8;

#ifdef _WIN32
    HDC m_screenDC = nullptr;
    HDC m_memoryDC = nullptr;
    HBITMAP m_bitmap = nullptr;
    BITMAPINFO m_bitmapInfo = {};
#endif
};

// ============================================================================
// DXGI Desktop Duplication Capture
// ============================================================================

class DXGICapture : public IScreenCapture
{
public:
    DXGICapture();
    ~DXGICapture() override;

    [[nodiscard]] bool initialize() override;
    void shutdown() override;
    [[nodiscard]] bool isInitialized() const override { return m_initialized; }
    [[nodiscard]] CaptureMethod getMethod() const override { return CaptureMethod::DXGI; }

    [[nodiscard]] Option<CapturedImage> captureScreen() override;
    [[nodiscard]] Option<CapturedImage> captureWindow(void* windowHandle) override;
    [[nodiscard]] Option<CapturedImage> captureRegion(const gui::Rect& region) override;

    void setFormat(CaptureFormat format) override { m_format = format; }
    [[nodiscard]] CaptureFormat getFormat() const override { return m_format; }

    /// Set which monitor to capture (0 = primary)
    void setMonitor(u32 monitorIndex) { m_monitorIndex = monitorIndex; }

private:
    bool initializeDuplication();
    void releaseDuplication();
    [[nodiscard]] Option<CapturedImage> acquireFrame();

private:
    bool m_initialized = false;
    CaptureFormat m_format = CaptureFormat::BGRA8;
    u32 m_monitorIndex = 0;

#ifdef _WIN32
    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGIOutputDuplication* m_duplication = nullptr;
    ID3D11Texture2D* m_stagingTexture = nullptr;
    u32 m_outputWidth = 0;
    u32 m_outputHeight = 0;
#endif
};

}  // namespace dakt::overlay
