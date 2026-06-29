// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ThemeComponentFactory.h
// Instantiates IComponent objects from parsed ThemeElement data.
//
// The factory is the bridge between the theme XML and the live component tree.
// It reads ThemeElement attributes and constructs the corresponding component
// with its config fully applied.
//
// Supported element types
// -----------------------
//   carousel     → CarouselComponent
//   image        → ImageComponent
//   text         → TextComponent
//
// New element types added in Clon3D format 2
// ------------------------------------------
//   overlay      → ImageComponent (rendered above all other elements)
//   statusbar    → TextComponent  (docked bar, usually clock / battery)
//   navigation   → custom NavigationComponent (future)
//   panel        → container for arbitrary child components
//   dashboard    → container for stats/info widgets
//   hud          → heads-up display layer (always on top)
//
// Unknown types are silently skipped; a warning is logged.
//
// Usage
// -----
//   ThemeComponentFactory factory(renderer, themeData);
//   auto components = factory.buildSystemViewComponents();
//   for (auto& c : components)
//       systemView.addChild(c);

#pragma once

#include "../components/IComponent.h"
#include "../components/CarouselComponent.h"
#include "ThemeData.h"

#include <memory>
#include <vector>

namespace clon3d {

class IRenderer;

/// Creates UI components from parsed theme elements.
class ThemeComponentFactory {
public:
    /// @param renderer  Renderer used to load textures.
    /// @param theme     The loaded theme data.
    ThemeComponentFactory(IRenderer& renderer, std::shared_ptr<ThemeData> theme);

    // -------------------------------------------------------------------------
    // High-level builders
    // -------------------------------------------------------------------------

    /// Build all components defined in the theme's <systemView> element.
    /// Returns an ordered list ready to be added as children of SystemView.
    std::vector<ComponentPtr> buildSystemViewComponents() const;

    /// Build a CarouselComponent from the theme's <carousel> element.
    /// Returns nullptr if the theme has no <carousel> element.
    std::shared_ptr<CarouselComponent> buildCarousel() const;

    // -------------------------------------------------------------------------
    // Low-level builders (used by the high-level builders or directly)
    // -------------------------------------------------------------------------

    /// Create a component from a single ThemeElement.
    /// Returns nullptr for unknown or unsupported element types.
    ComponentPtr buildComponent(const ThemeElement::Ptr& elem) const;

    /// Apply layout properties (pos, size) from an element to a component.
    static void applyLayout(IComponent& component, const ThemeElement::Ptr& elem);

private:
    /// Build a CarouselComponent from a <carousel> element.
    std::shared_ptr<CarouselComponent> buildCarouselFromElement(
        const ThemeElement::Ptr& elem) const;

    IRenderer&                 mRenderer;
    std::shared_ptr<ThemeData> mTheme;
};

} // namespace clon3d
