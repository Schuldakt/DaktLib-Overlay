#include "dakt/overlay/c_api.h"

#include <memory>

#include "dakt/overlay/core/OverlayManager.hpp"
#include "dakt/overlay/core/OverlayWindow.hpp"

struct DaktOverlayManager {
  std::unique_ptr<dakt::overlay::OverlayManager> impl;
};

struct DaktOverlayWindow {
  std::shared_ptr<dakt::overlay::OverlayWindow> impl;
};

extern "C" {

DaktOverlayManagerHandle dakt_overlay_create(void) {
  auto handle = new DaktOverlayManager{};
  handle->impl = std::make_unique<dakt::overlay::OverlayManager>();
  return handle;
}

void dakt_overlay_destroy(DaktOverlayManagerHandle mgr) {
  if (!mgr) {
    return;
  }
  delete mgr;
}

DaktOverlayWindowHandle
dakt_overlay_create_window(DaktOverlayManagerHandle mgr,
                           const DaktOverlayConfig *cfg) {
  if (!mgr) {
    return nullptr;
  }

  auto windowHandle = new DaktOverlayWindow{};
  windowHandle->impl = mgr->impl->createWindow();

  if (cfg) {
    windowHandle->impl->setBounds(cfg->x, cfg->y, cfg->width, cfg->height);
    windowHandle->impl->setOpacity(cfg->opacity);
    windowHandle->impl->setClickThrough(cfg->clickThrough != 0);
  }

  return windowHandle;
}

void dakt_overlay_destroy_window(DaktOverlayWindowHandle wnd) {
  if (!wnd) {
    return;
  }
  delete wnd;
}

void dakt_overlay_show(DaktOverlayWindowHandle wnd) {
  if (!wnd || !wnd->impl) {
    return;
  }
  wnd->impl->show();
}

void dakt_overlay_hide(DaktOverlayWindowHandle wnd) {
  if (!wnd || !wnd->impl) {
    return;
  }
  wnd->impl->hide();
}

void dakt_overlay_set_bounds(DaktOverlayWindowHandle wnd, int x, int y, int w,
                             int h) {
  if (!wnd || !wnd->impl) {
    return;
  }
  wnd->impl->setBounds(x, y, w, h);
}

void dakt_overlay_set_opacity(DaktOverlayWindowHandle wnd, float opacity) {
  if (!wnd || !wnd->impl) {
    return;
  }
  wnd->impl->setOpacity(opacity);
}

void dakt_overlay_set_click_through(DaktOverlayWindowHandle wnd, int enable) {
  if (!wnd || !wnd->impl) {
    return;
  }
  wnd->impl->setClickThrough(enable != 0);
}

} // extern "C"
