// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — CarouselComponent.cpp

#include "CarouselComponent.h"
#include "../core/Log.h"
#include <algorithm>
#include <cmath>

namespace clon3d {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

CarouselComponent::CarouselComponent(CarouselConfig config)
    : mConfig(std::move(config))
{
    // Sync cached bounds from config.
    mBounds = { mConfig.posX, mConfig.posY, mConfig.width, mConfig.height };
}

// ---------------------------------------------------------------------------
// Bounds
// ---------------------------------------------------------------------------

Rect CarouselComponent::bounds() const {
    return mBounds;
}

void CarouselComponent::setBounds(const Rect& r) {
    mBounds = r;
    mConfig.posX   = r.x;
    mConfig.posY   = r.y;
    mConfig.width  = r.w;
    mConfig.height = r.h;
}

// ---------------------------------------------------------------------------
// Data
// ---------------------------------------------------------------------------

void CarouselComponent::setSystems(std::vector<SystemDescriptor> systems) {
    mSystems       = std::move(systems);
    mLogoTextures.assign(mSystems.size(), INVALID_TEXTURE);
    mSelectedIndex = 0;
    mScrollOffset  = 0.0f;
    mScrollTarget  = 0.0f;
    mAnimating     = false;
}

void CarouselComponent::setSelectedIndex(size_t index) {
    if (mSystems.empty()) return;
    mSelectedIndex = index % mSystems.size();
    mScrollOffset  = static_cast<float>(mSelectedIndex);
    mScrollTarget  = mScrollOffset;
    mAnimating     = false;
}

void CarouselComponent::setConfig(CarouselConfig config) {
    mConfig = std::move(config);
    mBounds = { mConfig.posX, mConfig.posY, mConfig.width, mConfig.height };
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void CarouselComponent::update(float dt) {
    if (mAnimating) {
        const float speed = (mConfig.animationDuration > 0.0f)
                            ? (1.0f / mConfig.animationDuration) : 999.0f;
        float diff = mScrollTarget - mScrollOffset;

        // Handle wrap-around: choose shortest direction.
        if (mConfig.wrapAround && !mSystems.empty()) {
            float n = static_cast<float>(mSystems.size());
            if (diff >  n / 2.0f) diff -= n;
            if (diff < -n / 2.0f) diff += n;
        }

        float step = diff * speed * dt;
        if (std::abs(step) >= std::abs(diff)) {
            mScrollOffset = mScrollTarget;
            mAnimating    = false;
        } else {
            mScrollOffset += step;
        }

        // Keep offset in [0, n).
        if (!mSystems.empty()) {
            float n = static_cast<float>(mSystems.size());
            while (mScrollOffset <  0.0f) mScrollOffset += n;
            while (mScrollOffset >= n)    mScrollOffset -= n;
        }
    }

    updateChildren(dt);
}

// ---------------------------------------------------------------------------
// Rendering helpers
// ---------------------------------------------------------------------------

float CarouselComponent::itemScaleForOffset(float offset) const {
    // Scale falls off with distance from the selected centre slot.
    float normalised = std::min(std::abs(offset), 1.0f);
    return 1.0f + (mConfig.itemScale - 1.0f) * (1.0f - normalised);
}

Rect CarouselComponent::itemRect(float slotOffset) const {
    const bool horiz = (mConfig.orientation == CarouselConfig::Orientation::Horizontal);
    const float scale = itemScaleForOffset(slotOffset);

    // Carousel fills mBounds. The selected item occupies logoScale * height.
    const float itemH = mBounds.h * mConfig.logoScale * scale;
    const float itemW = itemH;  // square for now; textures are scaled to fit.

    const float centreX = mBounds.x + mBounds.w * 0.5f;
    const float centreY = mBounds.y + mBounds.h * 0.5f;

    float cx, cy;
    if (horiz) {
        cx = centreX + slotOffset * (mBounds.w * mConfig.itemSpacing);
        cy = centreY;
    } else {
        cx = centreX;
        cy = centreY + slotOffset * (mBounds.h * mConfig.itemSpacing);
    }

    return { cx - itemW * 0.5f, cy - itemH * 0.5f, itemW, itemH };
}

void CarouselComponent::renderSelectorBackground(IRenderer& renderer) {
    if (!mConfig.selectorVisible) return;
    // Centre selector rect — same position as slot 0.
    Rect selectorRect = itemRect(0.0f);
    // Enlarge slightly for visual breathing room.
    const float pad = 0.01f;
    selectorRect.x -= pad;
    selectorRect.y -= pad;
    selectorRect.w += pad * 2.0f;
    selectorRect.h += pad * 2.0f;
    renderer.drawRect(selectorRect, mConfig.selectorColor);
}

void CarouselComponent::renderItem(IRenderer& renderer, size_t sysIdx, float slotOffset) {
    Rect r = itemRect(slotOffset);
    // Clip to carousel bounds.
    if (r.right() < mBounds.x || r.x > mBounds.right()) return;

    TextureHandle tex = (sysIdx < mLogoTextures.size()) ? mLogoTextures[sysIdx] : INVALID_TEXTURE;

    if (tex != INVALID_TEXTURE) {
        renderer.drawTexture(tex, r);
    } else {
        // Fallback: draw a coloured placeholder rectangle.
        Color fg = { 180, 180, 180, 200 };
        renderer.drawRect(r, fg);
    }
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

void CarouselComponent::render(IRenderer& renderer) {
    if (!mVisible || mSystems.empty()) return;

    renderer.setClipRect(mBounds);

    renderSelectorBackground(renderer);

    const int n = static_cast<int>(mSystems.size());
    const int half = mConfig.maxVisibleItems / 2 + 1;

    // Draw items from furthest to closest so the selected item renders on top.
    for (int pass = half; pass >= 0; --pass) {
        for (int sign : {-1, 1}) {
            if (pass == 0 && sign == -1) continue; // don't double-draw centre

            int slot = pass * sign;
            // Wrap-around: compute actual system index.
            int sysIdx = static_cast<int>(std::round(mScrollOffset)) + slot;
            if (mConfig.wrapAround) {
                sysIdx = ((sysIdx % n) + n) % n;
            } else {
                if (sysIdx < 0 || sysIdx >= n) continue;
            }

            float slotOffset = static_cast<float>(slot) -
                               (mScrollOffset - std::round(mScrollOffset));
            renderItem(renderer, static_cast<size_t>(sysIdx), slotOffset);
        }
    }

    renderer.clearClipRect();
    renderChildren(renderer);
}

// ---------------------------------------------------------------------------
// Input handling
// ---------------------------------------------------------------------------

bool CarouselComponent::onInput(const InputEvent& event) {
    if (mSystems.empty()) return false;

    const bool horiz = (mConfig.orientation == CarouselConfig::Orientation::Horizontal);
    const bool nextAction = horiz ? (event.action == InputAction::Right)
                                  : (event.action == InputAction::Down);
    const bool prevAction = horiz ? (event.action == InputAction::Left)
                                  : (event.action == InputAction::Up);

    if (event.state == InputState::Pressed || event.state == InputState::Held) {
        if (nextAction) {
            size_t next = (mSelectedIndex + 1) % mSystems.size();
            if (!mConfig.wrapAround && next == 0) return true; // at end
            mSelectedIndex = next;
            mScrollTarget  = static_cast<float>(mSelectedIndex);
            mAnimating     = true;
            if (mHighlightCallback) mHighlightCallback(mSystems[mSelectedIndex]);
            return true;
        }
        if (prevAction) {
            size_t prev = (mSelectedIndex == 0) ? mSystems.size() - 1 : mSelectedIndex - 1;
            if (!mConfig.wrapAround && mSelectedIndex == 0) return true;
            mSelectedIndex = prev;
            mScrollTarget  = static_cast<float>(mSelectedIndex);
            mAnimating     = true;
            if (mHighlightCallback) mHighlightCallback(mSystems[mSelectedIndex]);
            return true;
        }
        if (event.action == InputAction::Accept) {
            if (mSelectCallback) mSelectCallback(mSystems[mSelectedIndex]);
            return true;
        }
    }

    return false;
}

} // namespace clon3d
