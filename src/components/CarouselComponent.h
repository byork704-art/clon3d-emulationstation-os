// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — CarouselComponent.h
// Configurable carousel used in the System View.
//
// This component replaces the original hardcoded carousel.  Every aspect of
// its appearance is driven by the theme XML so that theme authors have full
// control over how the system list is presented.
//
// Theme XML example
// -----------------
//   <carousel name="systemCarousel">
//     <orientation>horizontal</orientation>   <!-- horizontal | vertical -->
//     <pos x="0.0" y="0.3" />
//     <size w="1.0" h="0.4" />
//     <itemScale>1.2</itemScale>              <!-- scale of the selected item -->
//     <itemSpacing>0.2</itemSpacing>          <!-- normalised gap between items -->
//     <selectorVisible>false</selectorVisible><!-- hide the selector box -->
//     <selectorColor r="255" g="255" b="255" a="100" />
//     <logoScale>0.5</logoScale>
//     <animationDuration>0.3</animationDuration>
//   </carousel>
//
// Backward compatibility
// ----------------------
// When a legacy theme does not define a <carousel> element the engine
// falls back to default values that reproduce the original RetroPie behaviour.

#pragma once

#include "IComponent.h"
#include <functional>
#include <string>
#include <vector>

namespace clon3d {

// ---------------------------------------------------------------------------
// System descriptor (data model for one entry in the carousel)
// ---------------------------------------------------------------------------

/// Metadata for a single game system shown in the carousel.
struct SystemDescriptor {
    std::string id;              ///< Unique identifier (e.g. "nes").
    std::string name;            ///< Display name (e.g. "Nintendo Entertainment System").
    std::string logoPath;        ///< Path to the SVG/PNG logo image.
    std::string backgroundPath;  ///< Optional path to a per-system background image.
    size_t      gameCount{0};    ///< Number of games available.
};

// ---------------------------------------------------------------------------
// Carousel configuration (populated from theme XML)
// ---------------------------------------------------------------------------

/// Controls every visual aspect of the carousel.
/// All fields have defaults that reproduce the legacy carousel behaviour.
struct CarouselConfig {
    // Layout
    enum class Orientation { Horizontal, Vertical };
    Orientation orientation{Orientation::Horizontal};

    // Geometry (normalised screen coords)
    float posX{0.0f}, posY{0.33f};
    float width{1.0f}, height{0.4f};

    // Item appearance
    float itemScale{1.2f};      ///< Scale factor applied to the selected item.
    float itemSpacing{0.22f};   ///< Normalised gap between adjacent logo centres.
    float logoScale{0.5f};      ///< Logo size relative to carousel height.

    // Selector background (white box shown behind the selected item)
    bool  selectorVisible{true};
    Color selectorColor{255, 255, 255, 200};

    // Animation
    float animationDuration{0.3f}; ///< Seconds to scroll between systems.

    // Miscellaneous
    int   maxVisibleItems{5};   ///< Maximum items rendered simultaneously.
    bool  wrapAround{true};     ///< Wrap from last item back to first.
};

// ---------------------------------------------------------------------------
// CarouselComponent
// ---------------------------------------------------------------------------

/// A scrolling list of system logos.
///
/// The component is driven by a list of SystemDescriptors and a CarouselConfig.
/// It handles its own animation, input forwarding, and rendering.
///
/// Usage:
///   auto carousel = std::make_shared<CarouselComponent>(config, systems);
///   carousel->setName("systemCarousel");
///   systemView.addChild(carousel);
class CarouselComponent : public IComponent {
public:
    /// Callback type invoked when the user confirms a selection.
    using SelectCallback = std::function<void(const SystemDescriptor&)>;
    /// Callback type invoked whenever the highlighted system changes.
    using HighlightCallback = std::function<void(const SystemDescriptor&)>;

    // -------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------

    explicit CarouselComponent(CarouselConfig config = {});

    // -------------------------------------------------------------------------
    // IComponent interface
    // -------------------------------------------------------------------------

    const char* typeName() const override { return "carousel"; }

    Rect bounds() const override;
    void setBounds(const Rect& r) override;

    void update(float dt) override;
    void render(IRenderer& renderer) override;
    bool onInput(const InputEvent& event) override;

    // -------------------------------------------------------------------------
    // Data
    // -------------------------------------------------------------------------

    /// Replace the full system list.
    void setSystems(std::vector<SystemDescriptor> systems);

    /// Read-only access to the system list.
    const std::vector<SystemDescriptor>& systems() const { return mSystems; }

    /// Index of the currently highlighted system (0-based).
    size_t selectedIndex() const { return mSelectedIndex; }

    /// Set the selected index without animation.
    void setSelectedIndex(size_t index);

    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------

    const CarouselConfig& config() const { return mConfig; }

    /// Update the config at runtime (e.g. after theme reload).
    void setConfig(CarouselConfig config);

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    void setSelectCallback   (SelectCallback    cb) { mSelectCallback    = std::move(cb); }
    void setHighlightCallback(HighlightCallback cb) { mHighlightCallback = std::move(cb); }

private:
    // ------ rendering helpers -----------------------------------------------

    /// Returns the draw rect for item at position @p slot relative to centre.
    Rect itemRect(float slotOffset) const;

    /// Returns the scale for an item at the given offset from centre (0 = centre).
    float itemScaleForOffset(float offset) const;

    void renderSelectorBackground(IRenderer& renderer);
    void renderItem(IRenderer& renderer, size_t systemIndex, float slotOffset);

    // ------ state -----------------------------------------------------------

    CarouselConfig                mConfig;
    std::vector<SystemDescriptor> mSystems;
    size_t                        mSelectedIndex{0};

    // Textures (INVALID_TEXTURE until loaded)
    std::vector<TextureHandle>    mLogoTextures;

    // Animation
    float  mScrollOffset{0.0f};   ///< Current fractional position (0 = no scroll).
    float  mScrollTarget{0.0f};   ///< Target position for the ongoing animation.
    bool   mAnimating{false};

    // Callbacks
    SelectCallback    mSelectCallback;
    HighlightCallback mHighlightCallback;

    // Cached bounds (set via setBounds)
    Rect mBounds;
};

} // namespace clon3d
