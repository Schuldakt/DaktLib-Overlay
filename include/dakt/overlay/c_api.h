#pragma once

#include "core/Export.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DaktOverlayManager *DaktOverlayManagerHandle;
typedef struct DaktOverlayWindow *DaktOverlayWindowHandle;

typedef struct {
  int x;
  int y;
  int width;
  int height;
  float opacity;
  int clickThrough; /* bool */
} DaktOverlayConfig;

DAKT_OVERLAY_API DaktOverlayManagerHandle dakt_overlay_create(void);
DAKT_OVERLAY_API void dakt_overlay_destroy(DaktOverlayManagerHandle mgr);

DAKT_OVERLAY_API DaktOverlayWindowHandle dakt_overlay_create_window(
    DaktOverlayManagerHandle mgr, const DaktOverlayConfig *cfg);
DAKT_OVERLAY_API void dakt_overlay_destroy_window(DaktOverlayWindowHandle wnd);

DAKT_OVERLAY_API void dakt_overlay_show(DaktOverlayWindowHandle wnd);
DAKT_OVERLAY_API void dakt_overlay_hide(DaktOverlayWindowHandle wnd);
DAKT_OVERLAY_API void dakt_overlay_set_bounds(DaktOverlayWindowHandle wnd,
                                              int x, int y, int w, int h);
DAKT_OVERLAY_API void dakt_overlay_set_opacity(DaktOverlayWindowHandle wnd,
                                               float opacity);
DAKT_OVERLAY_API void
dakt_overlay_set_click_through(DaktOverlayWindowHandle wnd, int enable);

#ifdef __cplusplus
}
#endif
