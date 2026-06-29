// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ImageComponent.h
// Displays a static or animated image loaded from disk.

#pragma once

#include "IComponent.h"
#include <string>

namespace clon3d {

/// Renders a single texture within its bounds.
/// Scale modes:
///   - Stretch:  fill bounds exactly (may distort).
///   - Fit:      scale uniformly to fit within bounds (letterbox).
///   - Fill:     scale uniformly to cover bounds (crop edges).
///   - None:     render at native size, top-left aligned.
class ImageComponent : public IComponent {
public:
    enum class ScaleMode { Stretch, Fit, Fill, None };

    explicit ImageComponent(ScaleMode mode = ScaleMode::Fit);
    ~ImageComponent() override;

    // -------------------------------------------------------------------------
    // IComponent interface
    // -------------------------------------------------------------------------

    const char* typeName() const override { return "image"; }

    Rect bounds() const override { return mBounds; }
    void setBounds(const Rect& r) override { mBounds = r; }

    void update(float dt) override;
    void render(IRenderer& renderer) override;

    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------

    /// Load a texture from @p filePath.  Must be called after renderer is ready.
    void setImagePath(const std::string& filePath, IRenderer& renderer);

    /// Tint colour applied to the texture on draw.
    void setTint(Color tint) { mTint = tint; }
    Color tint() const { return mTint; }

    void setScaleMode(ScaleMode mode) { mScaleMode = mode; }

    // -------------------------------------------------------------------------
    // Animation
    // -------------------------------------------------------------------------

    /// Fade in/out.
    float opacity() const { return mOpacity; }
    void  setOpacity(float op) { mOpacity = op; }

private:
    Rect          mBounds;
    TextureHandle mTexture{INVALID_TEXTURE};
    Color         mTint{Color::white()};
    ScaleMode     mScaleMode{ScaleMode::Fit};
    float         mOpacity{1.0f};
    std::string   mFilePath;
    int           mTextureW{0}, mTextureH{0};
};

} // namespace clon3d
