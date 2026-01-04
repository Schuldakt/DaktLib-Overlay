#pragma once

namespace dakt::overlay {
class ISurface {
public:
  virtual ~ISurface() = default;

  virtual void resize(int width, int height) = 0;
  virtual void present() = 0;
  virtual void setOpacity(float value) = 0;
  virtual void setHitTest(bool enabled) = 0;
};

class SoftwareSurface final : public ISurface {
public:
  SoftwareSurface();
  ~SoftwareSurface() override;

  void resize(int width, int height) override;
  void present() override;
  void setOpacity(float value) override;
  void setHitTest(bool enabled) override;
};

class SwapchainSurface : public ISurface {
public:
  ~SwapchainSurface() override = default;
};
} // namespace dakt::overlay
