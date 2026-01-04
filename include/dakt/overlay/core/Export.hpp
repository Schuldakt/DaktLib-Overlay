#pragma once

#if defined(_WIN32)
#if defined(DAKT_OVERLAY_BUILD)
#define DAKT_OVERLAY_API __declspec(dllexport)
#else
#define DAKT_OVERLAY_API __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#define DAKT_OVERLAY_API __attribute__((visibility("default")))
#else
#define DAKT_OVERLAY_API
#endif
