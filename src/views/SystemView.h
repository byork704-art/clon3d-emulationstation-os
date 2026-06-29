// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — SystemView.h
// The system-selection view.
//
// Architecture overview
// ---------------------
// SystemView operates in one of two modes:
//
//   Legacy mode (default, backward-compatible)
//   ------------------------------------------
//   The view renders the system list with the hardcoded RetroPie-style
//   horizontal carousel.  No theme XML is required.  This matches the
//   behaviour of the original EmulationStation exactly.
//
//   Theme-controlled mode (new Clon3D feature)
//   ------------------------------------------
//   When the active theme contains a <systemView> element with a <carousel>
//   child (or has formatVersion >= 2), the view switches to theme-controlled
//   mode.  In this mode:
//
//   • The component tree is built entirely by ThemeComponentFactory.
//   • The default carousel is replaced by the theme-defined carousel.
//   • Additional elements (<image>, <overlay>, <statusbar>, etc.) are
//     rendered as specified by the theme.
//   • The selector background can be hidden or re-styled.
//
// Switching modes
// ---------------
// Call setTheme() after construction.  If the theme does not contain the
// required Clon3D elements the view stays in legacy mode automatically.
//
// Input
// -----
// In both modes input is forwarded to the carousel component.  In legacy
// mode the view owns the CarouselComponent; in theme-controlled mode it is
// one of the children built from the theme.

#pragma once

#include "../components/IComponent.h"
#include "../components/CarouselComponent.h"
#include "../theme/ThemeData.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace clon3d {

class IRenderer;
class ThemeComponentFactory;

// ---------------------------------------------------------------------------
// SystemView
// ---------------------------------------------------------------------------

/// The top-level view for selecting a game system.
class SystemView : public IComponent {
public:
    // -------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------

    SystemView();
    ~SystemView() override = default;

    // -------------------------------------------------------------------------
    // IComponent interface
    // -------------------------------------------------------------------------

    const char* typeName() const override { return "systemView"; }

    Rect bounds() const override { return mBounds; }
    void setBounds(const Rect& r) override;

    void update(float dt) override;
    void render(IRenderer& renderer) override;
    bool onInput(const InputEvent& event) override;

    // -------------------------------------------------------------------------
    // Initialisation
    // -------------------------------------------------------------------------

    /// Set the list of systems to display.
    void setSystems(std::vector<SystemDescriptor> systems);

    /// Apply a loaded theme.
    /// Switches to theme-controlled mode if the theme supports it.
    void setTheme(std::shared_ptr<ThemeData> theme, IRenderer& renderer);

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    using SystemSelectedCallback  = std::function<void(const SystemDescriptor&)>;
    using SystemHighlightCallback = std::function<void(const SystemDescriptor&)>;

    void setOnSystemSelected (SystemSelectedCallback  cb) { mOnSelected  = std::move(cb); }
    void setOnSystemHighlight(SystemHighlightCallback cb) { mOnHighlight = std::move(cb); }

    // -------------------------------------------------------------------------
    // Query
    // -------------------------------------------------------------------------

    /// True when the view is in theme-controlled mode.
    bool isThemeControlled() const { return mThemeControlled; }

    /// The currently highlighted system, or nullptr if none.
    const SystemDescriptor* currentSystem() const;

private:
    // ------ Mode switching --------------------------------------------------

    /// Build the legacy carousel component.
    void buildLegacyCarousel();

    /// Build the theme-controlled component tree.
    void buildThemeComponents(IRenderer& renderer);

    // ------ Legacy rendering helpers ----------------------------------------

    /// Render the default selector background rectangle.
    void renderLegacySelectorBackground(IRenderer& renderer);

    // ------ Data ------------------------------------------------------------

    Rect                          mBounds{0.0f, 0.0f, 1.0f, 1.0f};
    std::vector<SystemDescriptor> mSystems;
    std::shared_ptr<ThemeData>    mTheme;

    // Whether the view is driven by theme XML.
    bool mThemeControlled{false};

    // Components:
    //   In legacy mode:         mCarousel is the only child.
    //   In theme-controlled:    mChildren (from IComponent) holds all theme comps.
    std::shared_ptr<CarouselComponent> mCarousel; ///< The active carousel (both modes).

    // Callbacks
    SystemSelectedCallback  mOnSelected;
    SystemHighlightCallback mOnHighlight;
};

} // namespace clon3d
