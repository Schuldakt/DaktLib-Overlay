// ============================================================================
// DaktLib - Overlay Module - Screen Capture Implementation
// ============================================================================

#include <dakt/logger/Logger.hpp>
#include <dakt/overlay/ScreenCapture.hpp>

#include <algorithm>
#include <cstring>

namespace dakt::overlay
{

// ============================================================================
// Screen Capture Factory
// ============================================================================

std::unique_ptr<IScreenCapture> ScreenCapture::create(CaptureMethod method)
{
    if (method == CaptureMethod::Auto)
    {
        method = getBestMethod();
    }

    switch (method)
    {
        case CaptureMethod::DXGI:
            if (isMethodAvailable(CaptureMethod::DXGI))
            {
                return std::make_unique<DXGICapture>();
            }
            [[fallthrough]];

        case CaptureMethod::BitBlt:
        default:
            return std::make_unique<BitBltCapture>();
    }
}

bool ScreenCapture::isMethodAvailable(CaptureMethod method)
{
#ifdef _WIN32
    switch (method)
    {
        case CaptureMethod::BitBlt:
            return true;  // Always available

        case CaptureMethod::DXGI:
            // DXGI Desktop Duplication requires Windows 8+
            {
                OSVERSIONINFOEXW osvi = {};
                osvi.dwOSVersionInfoSize = sizeof(osvi);
                osvi.dwMajorVersion = 6;
                osvi.dwMinorVersion = 2;  // Windows 8

                DWORDLONG conditionMask = 0;
                VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
                VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

                return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, conditionMask) != FALSE;
            }

        case CaptureMethod::WGC:
            // Windows Graphics Capture requires Windows 10 1903+
            // Simplified check - would need proper version detection
            return false;

        default:
            return false;
    }
#else
    (void)method;
    return false;
#endif
}

CaptureMethod ScreenCapture::getBestMethod()
{
    if (isMethodAvailable(CaptureMethod::DXGI))
    {
        return CaptureMethod::DXGI;
    }
    return CaptureMethod::BitBlt;
}

Option<CapturedImage> ScreenCapture::convertFormat(const CapturedImage& image, CaptureFormat targetFormat)
{
    if (image.format == targetFormat)
    {
        return image;  // No conversion needed
    }

    CapturedImage result;
    result.width = image.width;
    result.height = image.height;
    result.format = targetFormat;
    result.timestamp = image.timestamp;

    usize pixelCount = static_cast<usize>(image.width) * image.height;

    if (image.format == CaptureFormat::BGRA8)
    {
        if (targetFormat == CaptureFormat::RGBA8)
        {
            result.stride = image.width * 4;
            result.data.resize(pixelCount * 4);

            for (usize i = 0; i < pixelCount; ++i)
            {
                result.data[i * 4 + 0] = image.data[i * 4 + 2];  // R
                result.data[i * 4 + 1] = image.data[i * 4 + 1];  // G
                result.data[i * 4 + 2] = image.data[i * 4 + 0];  // B
                result.data[i * 4 + 3] = image.data[i * 4 + 3];  // A
            }
        }
        else if (targetFormat == CaptureFormat::RGB8)
        {
            result.stride = image.width * 3;
            result.data.resize(pixelCount * 3);

            for (usize i = 0; i < pixelCount; ++i)
            {
                result.data[i * 3 + 0] = image.data[i * 4 + 2];  // R
                result.data[i * 3 + 1] = image.data[i * 4 + 1];  // G
                result.data[i * 3 + 2] = image.data[i * 4 + 0];  // B
            }
        }
        else if (targetFormat == CaptureFormat::Grayscale)
        {
            result.stride = image.width;
            result.data.resize(pixelCount);

            for (usize i = 0; i < pixelCount; ++i)
            {
                // ITU-R BT.601 conversion
                u8 b = static_cast<u8>(image.data[i * 4 + 0]);
                u8 g = static_cast<u8>(image.data[i * 4 + 1]);
                u8 r = static_cast<u8>(image.data[i * 4 + 2]);

                result.data[i] = static_cast<byte>((r * 299 + g * 587 + b * 114) / 1000);
            }
        }
        else
        {
            return std::nullopt;
        }
    }
    else if (image.format == CaptureFormat::RGBA8)
    {
        if (targetFormat == CaptureFormat::BGRA8)
        {
            result.stride = image.width * 4;
            result.data.resize(pixelCount * 4);

            for (usize i = 0; i < pixelCount; ++i)
            {
                result.data[i * 4 + 0] = image.data[i * 4 + 2];  // B
                result.data[i * 4 + 1] = image.data[i * 4 + 1];  // G
                result.data[i * 4 + 2] = image.data[i * 4 + 0];  // R
                result.data[i * 4 + 3] = image.data[i * 4 + 3];  // A
            }
        }
        else if (targetFormat == CaptureFormat::Grayscale)
        {
            result.stride = image.width;
            result.data.resize(pixelCount);

            for (usize i = 0; i < pixelCount; ++i)
            {
                u8 r = static_cast<u8>(image.data[i * 4 + 0]);
                u8 g = static_cast<u8>(image.data[i * 4 + 1]);
                u8 b = static_cast<u8>(image.data[i * 4 + 2]);

                result.data[i] = static_cast<byte>((r * 299 + g * 587 + b * 114) / 1000);
            }
        }
        else
        {
            return std::nullopt;
        }
    }
    else
    {
        return std::nullopt;
    }

    return result;
}

Option<CapturedImage> ScreenCapture::scale(const CapturedImage& image, u32 newWidth, u32 newHeight)
{
    if (newWidth == 0 || newHeight == 0)
    {
        return std::nullopt;
    }

    if (image.format != CaptureFormat::BGRA8 && image.format != CaptureFormat::RGBA8)
    {
        return std::nullopt;  // Only support 32-bit formats for now
    }

    CapturedImage result;
    result.width = newWidth;
    result.height = newHeight;
    result.format = image.format;
    result.stride = newWidth * 4;
    result.timestamp = image.timestamp;
    result.data.resize(static_cast<usize>(newWidth) * newHeight * 4);

    // Simple bilinear interpolation
    f32 xRatio = static_cast<f32>(image.width - 1) / newWidth;
    f32 yRatio = static_cast<f32>(image.height - 1) / newHeight;

    for (u32 y = 0; y < newHeight; ++y)
    {
        for (u32 x = 0; x < newWidth; ++x)
        {
            f32 srcX = x * xRatio;
            f32 srcY = y * yRatio;

            u32 x0 = static_cast<u32>(srcX);
            u32 y0 = static_cast<u32>(srcY);
            u32 x1 = std::min(x0 + 1, image.width - 1);
            u32 y1 = std::min(y0 + 1, image.height - 1);

            f32 xFrac = srcX - x0;
            f32 yFrac = srcY - y0;

            for (u32 c = 0; c < 4; ++c)
            {
                f32 p00 = static_cast<f32>(static_cast<u8>(image.data[(y0 * image.width + x0) * 4 + c]));
                f32 p10 = static_cast<f32>(static_cast<u8>(image.data[(y0 * image.width + x1) * 4 + c]));
                f32 p01 = static_cast<f32>(static_cast<u8>(image.data[(y1 * image.width + x0) * 4 + c]));
                f32 p11 = static_cast<f32>(static_cast<u8>(image.data[(y1 * image.width + x1) * 4 + c]));

                f32 value = p00 * (1 - xFrac) * (1 - yFrac) + p10 * xFrac * (1 - yFrac) + p01 * (1 - xFrac) * yFrac +
                            p11 * xFrac * yFrac;

                result.data[(y * newWidth + x) * 4 + c] =
                    static_cast<byte>(std::clamp(static_cast<int>(value + 0.5f), 0, 255));
            }
        }
    }

    return result;
}

Option<CapturedImage> ScreenCapture::crop(const CapturedImage& image, const Rect& region)
{
    i32 x = static_cast<i32>(region.x());
    i32 y = static_cast<i32>(region.y());
    i32 w = static_cast<i32>(region.width());
    i32 h = static_cast<i32>(region.height());

    // Clamp to image bounds
    x = std::max(0, x);
    y = std::max(0, y);
    w = std::min(w, static_cast<i32>(image.width) - x);
    h = std::min(h, static_cast<i32>(image.height) - y);

    if (w <= 0 || h <= 0)
    {
        return std::nullopt;
    }

    u32 bytesPerPixel = 0;
    switch (image.format)
    {
        case CaptureFormat::BGRA8:
        case CaptureFormat::RGBA8:
            bytesPerPixel = 4;
            break;
        case CaptureFormat::RGB8:
            bytesPerPixel = 3;
            break;
        case CaptureFormat::Grayscale:
            bytesPerPixel = 1;
            break;
    }

    CapturedImage result;
    result.width = static_cast<u32>(w);
    result.height = static_cast<u32>(h);
    result.format = image.format;
    result.stride = result.width * bytesPerPixel;
    result.timestamp = image.timestamp;
    result.data.resize(static_cast<usize>(result.stride) * h);

    for (i32 row = 0; row < h; ++row)
    {
        const byte* srcRow = image.data.data() + (y + row) * image.stride + x * bytesPerPixel;
        byte* dstRow = result.data.data() + row * result.stride;
        std::memcpy(dstRow, srcRow, result.stride);
    }

    return result;
}

// ============================================================================
// BitBlt Capture Implementation
// ============================================================================

BitBltCapture::BitBltCapture() = default;

BitBltCapture::~BitBltCapture()
{
    shutdown();
}

bool BitBltCapture::initialize()
{
    if (m_initialized)
    {
        return true;
    }

#ifdef _WIN32
    m_screenDC = GetDC(nullptr);
    if (!m_screenDC)
    {
        DAKT_LOG_ERROR("Failed to get screen DC");
        return false;
    }

    m_memoryDC = CreateCompatibleDC(m_screenDC);
    if (!m_memoryDC)
    {
        ReleaseDC(nullptr, m_screenDC);
        m_screenDC = nullptr;
        DAKT_LOG_ERROR("Failed to create memory DC");
        return false;
    }

    m_initialized = true;
    DAKT_LOG_DEBUG("BitBlt capture initialized");
    return true;
#else
    return false;
#endif
}

void BitBltCapture::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

#ifdef _WIN32
    if (m_bitmap)
    {
        DeleteObject(m_bitmap);
        m_bitmap = nullptr;
    }

    if (m_memoryDC)
    {
        DeleteDC(m_memoryDC);
        m_memoryDC = nullptr;
    }

    if (m_screenDC)
    {
        ReleaseDC(nullptr, m_screenDC);
        m_screenDC = nullptr;
    }
#endif

    m_initialized = false;
    DAKT_LOG_DEBUG("BitBlt capture shutdown");
}

Option<CapturedImage> BitBltCapture::captureScreen()
{
#ifdef _WIN32
    if (!m_initialized)
    {
        return std::nullopt;
    }

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    Rect region{0, 0, static_cast<f32>(width), static_cast<f32>(height)};
    return captureRegion(region);
#else
    return std::nullopt;
#endif
}

Option<CapturedImage> BitBltCapture::captureWindow(void* windowHandle)
{
#ifdef _WIN32
    if (!m_initialized || !windowHandle)
    {
        return std::nullopt;
    }

    HWND hwnd = static_cast<HWND>(windowHandle);
    if (!IsWindow(hwnd))
    {
        return std::nullopt;
    }

    RECT rect;
    if (!GetWindowRect(hwnd, &rect))
    {
        return std::nullopt;
    }

    Rect region{static_cast<f32>(rect.left), static_cast<f32>(rect.top), static_cast<f32>(rect.right - rect.left),
                     static_cast<f32>(rect.bottom - rect.top)};

    return captureRegion(region);
#else
    (void)windowHandle;
    return std::nullopt;
#endif
}

Option<CapturedImage> BitBltCapture::captureRegion(const Rect& region)
{
#ifdef _WIN32
    if (!m_initialized)
    {
        return std::nullopt;
    }

    int x = static_cast<int>(region.x());
    int y = static_cast<int>(region.y());
    int width = static_cast<int>(region.width());
    int height = static_cast<int>(region.height());

    if (width <= 0 || height <= 0)
    {
        return std::nullopt;
    }

    // Create or recreate bitmap if size changed
    bool needNewBitmap = m_bitmap == nullptr;
    if (m_bitmap)
    {
        BITMAP bm;
        GetObject(m_bitmap, sizeof(bm), &bm);
        if (bm.bmWidth != width || bm.bmHeight != height)
        {
            DeleteObject(m_bitmap);
            m_bitmap = nullptr;
            needNewBitmap = true;
        }
    }

    if (needNewBitmap)
    {
        m_bitmapInfo = {};
        m_bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        m_bitmapInfo.bmiHeader.biWidth = width;
        m_bitmapInfo.bmiHeader.biHeight = -height;  // Top-down
        m_bitmapInfo.bmiHeader.biPlanes = 1;
        m_bitmapInfo.bmiHeader.biBitCount = 32;
        m_bitmapInfo.bmiHeader.biCompression = BI_RGB;

        m_bitmap = CreateCompatibleBitmap(m_screenDC, width, height);
        if (!m_bitmap)
        {
            return std::nullopt;
        }
    }

    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(m_memoryDC, m_bitmap));

    // Capture
    if (!BitBlt(m_memoryDC, 0, 0, width, height, m_screenDC, x, y, SRCCOPY))
    {
        SelectObject(m_memoryDC, oldBitmap);
        return std::nullopt;
    }

    SelectObject(m_memoryDC, oldBitmap);

    // Get pixels
    CapturedImage result;
    result.width = static_cast<u32>(width);
    result.height = static_cast<u32>(height);
    result.stride = static_cast<u32>(width * 4);
    result.format = CaptureFormat::BGRA8;
    result.data.resize(static_cast<usize>(width) * height * 4);

    GetDIBits(m_memoryDC, m_bitmap, 0, height, result.data.data(), &m_bitmapInfo, DIB_RGB_COLORS);

    // Convert to requested format if needed
    if (m_format != CaptureFormat::BGRA8)
    {
        auto converted = ScreenCapture::convertFormat(result, m_format);
        if (converted)
        {
            return converted;
        }
    }

    return result;
#else
    (void)region;
    return std::nullopt;
#endif
}

// ============================================================================
// DXGI Capture Implementation
// ============================================================================

DXGICapture::DXGICapture() = default;

DXGICapture::~DXGICapture()
{
    shutdown();
}

bool DXGICapture::initialize()
{
    if (m_initialized)
    {
        return true;
    }

#ifdef _WIN32
    // Create D3D11 device
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
        DAKT_LOG_ERROR("Failed to create D3D11 device");
        return false;
    }

    if (!initializeDuplication())
    {
        m_context->Release();
        m_device->Release();
        m_device = nullptr;
        m_context = nullptr;
        return false;
    }

    m_initialized = true;
    DAKT_LOG_DEBUG("DXGI capture initialized");
    return true;
#else
    return false;
#endif
}

void DXGICapture::shutdown()
{
    if (!m_initialized)
    {
        return;
    }

#ifdef _WIN32
    releaseDuplication();

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
#endif

    m_initialized = false;
    DAKT_LOG_DEBUG("DXGI capture shutdown");
}

Option<CapturedImage> DXGICapture::captureScreen()
{
    return acquireFrame();
}

Option<CapturedImage> DXGICapture::captureWindow(void* windowHandle)
{
    // DXGI captures the entire output, then we can crop
    auto fullCapture = captureScreen();
    if (!fullCapture)
    {
        return std::nullopt;
    }

#ifdef _WIN32
    HWND hwnd = static_cast<HWND>(windowHandle);
    if (!IsWindow(hwnd))
    {
        return fullCapture;
    }

    RECT rect;
    if (!GetWindowRect(hwnd, &rect))
    {
        return fullCapture;
    }

    Rect region{static_cast<f32>(rect.left), static_cast<f32>(rect.top), static_cast<f32>(rect.right - rect.left),
                     static_cast<f32>(rect.bottom - rect.top)};

    return ScreenCapture::crop(*fullCapture, region);
#else
    (void)windowHandle;
    return fullCapture;
#endif
}

Option<CapturedImage> DXGICapture::captureRegion(const Rect& region)
{
    auto fullCapture = captureScreen();
    if (!fullCapture)
    {
        return std::nullopt;
    }

    return ScreenCapture::crop(*fullCapture, region);
}

#ifdef _WIN32

bool DXGICapture::initializeDuplication()
{
    // Get DXGI device
    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (FAILED(hr))
    {
        return false;
    }

    // Get adapter
    IDXGIAdapter* adapter = nullptr;
    hr = dxgiDevice->GetAdapter(&adapter);
    dxgiDevice->Release();
    if (FAILED(hr))
    {
        return false;
    }

    // Get output
    IDXGIOutput* output = nullptr;
    hr = adapter->EnumOutputs(m_monitorIndex, &output);
    adapter->Release();
    if (FAILED(hr))
    {
        DAKT_LOG_ERROR("Failed to get output");
        return false;
    }

    // Get output1 for duplication
    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&output1));
    output->Release();
    if (FAILED(hr))
    {
        DAKT_LOG_ERROR("Failed to get IDXGIOutput1");
        return false;
    }

    // Create duplication
    hr = output1->DuplicateOutput(m_device, &m_duplication);
    output1->Release();
    if (FAILED(hr))
    {
        DAKT_LOG_ERROR("Failed to duplicate output");
        return false;
    }

    // Get output description
    DXGI_OUTDUPL_DESC desc;
    m_duplication->GetDesc(&desc);
    m_outputWidth = desc.ModeDesc.Width;
    m_outputHeight = desc.ModeDesc.Height;

    // Create staging texture
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = m_outputWidth;
    texDesc.Height = m_outputHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    hr = m_device->CreateTexture2D(&texDesc, nullptr, &m_stagingTexture);
    if (FAILED(hr))
    {
        m_duplication->Release();
        m_duplication = nullptr;
        return false;
    }

    return true;
}

void DXGICapture::releaseDuplication()
{
    if (m_stagingTexture)
    {
        m_stagingTexture->Release();
        m_stagingTexture = nullptr;
    }

    if (m_duplication)
    {
        m_duplication->Release();
        m_duplication = nullptr;
    }
}

Option<CapturedImage> DXGICapture::acquireFrame()
{
    if (!m_duplication)
    {
        return std::nullopt;
    }

    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* resource = nullptr;

    // Try to acquire frame
    HRESULT hr = m_duplication->AcquireNextFrame(100, &frameInfo, &resource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT)
    {
        return std::nullopt;  // No new frame
    }

    if (hr == DXGI_ERROR_ACCESS_LOST)
    {
        // Need to recreate duplication
        releaseDuplication();
        if (!initializeDuplication())
        {
            return std::nullopt;
        }
        hr = m_duplication->AcquireNextFrame(100, &frameInfo, &resource);
    }

    if (FAILED(hr))
    {
        return std::nullopt;
    }

    // Get texture
    ID3D11Texture2D* texture = nullptr;
    hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
    resource->Release();

    if (FAILED(hr))
    {
        m_duplication->ReleaseFrame();
        return std::nullopt;
    }

    // Copy to staging texture
    m_context->CopyResource(m_stagingTexture, texture);
    texture->Release();

    // Map staging texture
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_context->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr))
    {
        m_duplication->ReleaseFrame();
        return std::nullopt;
    }

    // Copy data
    CapturedImage result;
    result.width = m_outputWidth;
    result.height = m_outputHeight;
    result.stride = m_outputWidth * 4;
    result.format = CaptureFormat::BGRA8;
    result.data.resize(static_cast<usize>(result.stride) * m_outputHeight);

    const byte* src = static_cast<const byte*>(mapped.pData);
    for (u32 row = 0; row < m_outputHeight; ++row)
    {
        std::memcpy(result.data.data() + row * result.stride, src + row * mapped.RowPitch, result.stride);
    }

    m_context->Unmap(m_stagingTexture, 0);
    m_duplication->ReleaseFrame();

    // Convert if needed
    if (m_format != CaptureFormat::BGRA8)
    {
        auto converted = ScreenCapture::convertFormat(result, m_format);
        if (converted)
        {
            return converted;
        }
    }

    return result;
}

#else  // Non-Windows stubs

bool DXGICapture::initializeDuplication()
{
    return false;
}
void DXGICapture::releaseDuplication() {}
Option<CapturedImage> DXGICapture::acquireFrame()
{
    return std::nullopt;
}

#endif

}  // namespace dakt::overlay
