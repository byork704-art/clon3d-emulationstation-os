// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ThemeComponentFactory.cpp

#include "ThemeComponentFactory.h"
#include "../components/ImageComponent.h"
#include "../components/TextComponent.h"
#include "../core/Log.h"

#include <stdexcept>

namespace clon3d {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ThemeComponentFactory::ThemeComponentFactory(IRenderer& renderer,
                                              std::shared_ptr<ThemeData> theme)
    : mRenderer(renderer)
    , mTheme(std::move(theme))
{}

// ---------------------------------------------------------------------------
// Layout helper
// ---------------------------------------------------------------------------

void ThemeComponentFactory::applyLayout(IComponent& component,
                                         const ThemeElement::Ptr& elem) {
    auto [px, py] = elem->childPos("pos",  0.0f, 0.0f);
    auto [sw, sh] = elem->childSize("size", 1.0f, 1.0f);
    component.setBounds({ px, py, sw, sh });
}

// ---------------------------------------------------------------------------
// CarouselComponent builder
// ---------------------------------------------------------------------------

std::shared_ptr<CarouselComponent>
ThemeComponentFactory::buildCarouselFromElement(const ThemeElement::Ptr& elem) const {
    CarouselConfig cfg;

    // Orientation
    std::string orient = elem->childText("orientation", "horizontal");
    if (orient == "vertical")
        cfg.orientation = CarouselConfig::Orientation::Vertical;

    // Position / size
    auto [px, py] = elem->childPos();
    auto [sw, sh] = elem->childSize();
    cfg.posX = px; cfg.posY = py;
    cfg.width = sw; cfg.height = sh;

    // Item appearance
    cfg.itemScale    = elem->childFloat("itemScale",    1.2f);
    cfg.itemSpacing  = elem->childFloat("itemSpacing",  0.22f);
    cfg.logoScale    = elem->childFloat("logoScale",    0.5f);

    // Selector
    cfg.selectorVisible = elem->childBool("selectorVisible", true);
    cfg.selectorColor   = elem->childColor("selectorColor", {255, 255, 255, 200});

    // Animation
    cfg.animationDuration = elem->childFloat("animationDuration", 0.3f);

    // Misc
    cfg.maxVisibleItems = elem->childInt("maxVisibleItems", 5);
    cfg.wrapAround      = elem->childBool("wrapAround",     true);

    auto carousel = std::make_shared<CarouselComponent>(std::move(cfg));
    carousel->setName(elem->name);
    return carousel;
}

// ---------------------------------------------------------------------------
// Generic component builder
// ---------------------------------------------------------------------------

ComponentPtr ThemeComponentFactory::buildComponent(const ThemeElement::Ptr& elem) const {
    if (!elem) return nullptr;

    const std::string& type = elem->type;

    // ---- carousel ----------------------------------------------------------
    if (type == "carousel") {
        return buildCarouselFromElement(elem);
    }

    // ---- image / overlay / hud background (all become ImageComponent) ------
    if (type == "image" || type == "overlay" || type == "hud") {
        auto img = std::make_shared<ImageComponent>();
        img->setName(elem->name);
        applyLayout(*img, elem);

        std::string path = mTheme->resolvePath(elem->childText("path"));
        if (!path.empty()) {
            img->setImagePath(path, mRenderer);
        }

        // Tint / opacity
        Color tint = elem->childColor("color", Color::white());
        img->setTint(tint);

        float opacity = elem->childFloat("opacity", 1.0f);
        img->setOpacity(opacity);

        return img;
    }

    // ---- text / statusbar --------------------------------------------------
    if (type == "text" || type == "statusbar") {
        auto txt = std::make_shared<TextComponent>();
        txt->setName(elem->name);
        applyLayout(*txt, elem);

        txt->setText(elem->childText("value"));

        // Colour
        Color c = elem->childColor("color", Color::white());
        txt->setColor(c);

        // Font size (normalised)
        float fontSize = elem->childFloat("fontSize", 0.05f);
        txt->setFontSize(fontSize);

        // Alignment
        std::string ha = elem->childText("alignment", "center");
        if      (ha == "left")  txt->setHAlign(HAlign::Left);
        else if (ha == "right") txt->setHAlign(HAlign::Right);
        else                    txt->setHAlign(HAlign::Centre);

        txt->setWrap(elem->childBool("wrap", false));

        return txt;
    }

    // ---- panel / dashboard / navigation — container elements ----------------
    // These become plain IComponent containers.  Their children are built
    // recursively and attached.
    if (type == "panel" || type == "dashboard" || type == "navigation") {
        // Use an ImageComponent as a neutral container (transparent by default).
        auto container = std::make_shared<ImageComponent>();
        container->setName(elem->name);
        applyLayout(*container, elem);
        // Make the container transparent so it acts as a pure layout grouping.
        container->setTint({0, 0, 0, 0});

        for (auto& childElem : elem->children) {
            auto childComp = buildComponent(childElem);
            if (childComp) container->addChild(childComp);
        }

        return container;
    }

    // Unknown element — skip silently (only warn in debug mode).
    LOG_DEBUG("ThemeComponentFactory") << "skipping unknown element type: " << type;
    return nullptr;
}

// ---------------------------------------------------------------------------
// High-level builders
// ---------------------------------------------------------------------------

std::shared_ptr<CarouselComponent> ThemeComponentFactory::buildCarousel() const {
    if (!mTheme || !mTheme->hasThemeControlledCarousel()) return nullptr;

    auto carouselElem = mTheme->carouselElement();
    if (!carouselElem) return nullptr;

    return buildCarouselFromElement(carouselElem);
}

std::vector<ComponentPtr> ThemeComponentFactory::buildSystemViewComponents() const {
    std::vector<ComponentPtr> result;
    if (!mTheme) return result;

    auto sv = mTheme->systemView();
    if (!sv) return result;

    for (auto& childElem : sv->children) {
        auto comp = buildComponent(childElem);
        if (comp) result.push_back(comp);
    }

    return result;
}

} // namespace clon3d
