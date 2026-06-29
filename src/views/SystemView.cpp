// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — SystemView.cpp

#include "SystemView.h"
#include "../theme/ThemeComponentFactory.h"
#include "../core/Log.h"
#include <algorithm>

namespace clon3d {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

SystemView::SystemView() {
    // Starts in legacy mode.  buildLegacyCarousel() is called from setSystems().
}

// ---------------------------------------------------------------------------
// Bounds
// ---------------------------------------------------------------------------

void SystemView::setBounds(const Rect& r) {
    mBounds = r;
    // Propagate to all children.
    if (mCarousel) {
        if (mThemeControlled) {
            // In theme-controlled mode the carousel already has its own bounds
            // from the theme XML; only update if it spans the full screen.
            // (Theme authors usually set explicit bounds.)
        } else {
            // Legacy: carousel fills the full screen.
            mCarousel->setBounds(mBounds);
        }
    }
    for (auto& c : children())
        c->setBounds(r); // only when child doesn't have its own layout
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

void SystemView::setSystems(std::vector<SystemDescriptor> systems) {
    mSystems = std::move(systems);

    if (mCarousel) {
        mCarousel->setSystems(mSystems);
    } else {
        // First call: build legacy carousel now; theme may replace it later
        // via setTheme().
        buildLegacyCarousel();
    }

    LOG_INFO("SystemView") << "loaded " << mSystems.size() << " system(s)";
}

void SystemView::setTheme(std::shared_ptr<ThemeData> theme, IRenderer& renderer) {
    mTheme = std::move(theme);

    if (!mTheme) {
        LOG_WARN("SystemView") << "null theme passed — staying in legacy mode";
        return;
    }

    if (mTheme->hasThemeControlledCarousel()) {
        LOG_INFO("SystemView") << "switching to theme-controlled mode";
        buildThemeComponents(renderer);
    } else {
        LOG_INFO("SystemView") << "theme is legacy-compatible — using default carousel";
        buildLegacyCarousel();
    }
}

// ---------------------------------------------------------------------------
// Legacy carousel
// ---------------------------------------------------------------------------

void SystemView::buildLegacyCarousel() {
    // Default RetroPie-style horizontal carousel.
    CarouselConfig cfg;
    cfg.orientation      = CarouselConfig::Orientation::Horizontal;
    cfg.posX             = 0.0f;
    cfg.posY             = 0.33f;
    cfg.width            = 1.0f;
    cfg.height           = 0.4f;
    cfg.itemScale        = 1.2f;
    cfg.itemSpacing      = 0.22f;
    cfg.logoScale        = 0.5f;
    cfg.selectorVisible  = true;
    cfg.selectorColor    = { 255, 255, 255, 200 };
    cfg.animationDuration = 0.3f;
    cfg.maxVisibleItems  = 5;
    cfg.wrapAround       = true;

    mCarousel = std::make_shared<CarouselComponent>(std::move(cfg));
    mCarousel->setName("legacyCarousel");
    mCarousel->setSystems(mSystems);
    mCarousel->setSelectCallback([this](const SystemDescriptor& s) {
        if (mOnSelected) mOnSelected(s);
    });
    mCarousel->setHighlightCallback([this](const SystemDescriptor& s) {
        if (mOnHighlight) mOnHighlight(s);
    });

    // Legacy mode: clear any theme children and replace with just the carousel.
    clearChildren();
    addChild(mCarousel);
    mThemeControlled = false;

    LOG_DEBUG("SystemView") << "legacy carousel built";
}

// ---------------------------------------------------------------------------
// Theme-controlled component tree
// ---------------------------------------------------------------------------

void SystemView::buildThemeComponents(IRenderer& renderer) {
    ThemeComponentFactory factory(renderer, mTheme);

    // Build all components defined by the theme.
    auto comps = factory.buildSystemViewComponents();

    clearChildren();
    mCarousel = nullptr;

    for (auto& comp : comps) {
        // Locate the carousel component so we can wire callbacks.
        if (comp->typeName() == std::string("carousel")) {
            mCarousel = std::dynamic_pointer_cast<CarouselComponent>(comp);
        }
        addChild(comp);
    }

    // If the theme didn't provide a carousel, fall back to legacy.
    if (!mCarousel) {
        LOG_WARN("SystemView") << "theme has no carousel element — "
                                  "inserting default carousel as fallback";
        buildLegacyCarousel();
        return;
    }

    // Wire callbacks and populate systems.
    mCarousel->setSystems(mSystems);
    mCarousel->setSelectCallback([this](const SystemDescriptor& s) {
        if (mOnSelected) mOnSelected(s);
    });
    mCarousel->setHighlightCallback([this](const SystemDescriptor& s) {
        if (mOnHighlight) mOnHighlight(s);
    });

    mThemeControlled = true;
    LOG_INFO("SystemView") << "theme-controlled component tree built ("
                           << comps.size() << " top-level component(s))";
}

// ---------------------------------------------------------------------------
// Update / render / input
// ---------------------------------------------------------------------------

void SystemView::update(float dt) {
    updateChildren(dt);
}

void SystemView::render(IRenderer& renderer) {
    if (!mVisible) return;

    // In legacy mode: render the legacy selector background before the carousel.
    if (!mThemeControlled && mCarousel && mCarousel->config().selectorVisible) {
        renderLegacySelectorBackground(renderer);
    }

    renderChildren(renderer);
}

bool SystemView::onInput(const InputEvent& event) {
    // Forward input to carousel first; other children receive it if not consumed.
    if (mCarousel && mCarousel->onInput(event)) return true;

    for (auto& c : children()) {
        if (c.get() == mCarousel.get()) continue; // already tried above
        if (c->onInput(event)) return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Legacy selector background
// ---------------------------------------------------------------------------

void SystemView::renderLegacySelectorBackground(IRenderer& renderer) {
    // Draw the classic white semi-transparent rectangle behind the selected item.
    const auto& cfg = mCarousel->config();
    Rect selectorRect{
        cfg.posX + cfg.width * 0.5f - cfg.height * cfg.logoScale * 0.5f - 0.01f,
        cfg.posY + 0.01f,
        cfg.height * cfg.logoScale + 0.02f,
        cfg.height - 0.02f
    };
    renderer.drawRect(selectorRect, cfg.selectorColor);
}

// ---------------------------------------------------------------------------
// Query
// ---------------------------------------------------------------------------

const SystemDescriptor* SystemView::currentSystem() const {
    if (!mCarousel || mCarousel->systems().empty()) return nullptr;
    return &mCarousel->systems()[mCarousel->selectedIndex()];
}

} // namespace clon3d
