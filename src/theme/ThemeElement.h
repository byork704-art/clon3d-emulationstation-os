// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ThemeElement.h
// Data structures that represent one parsed XML element from a theme file.
//
// The theme loader (ThemeData) reads the XML and produces a tree of
// ThemeElements.  The view layer and component factory consume this tree to
// build the live UI.
//
// Design principle
// ----------------
// ThemeElement is intentionally a dumb data bag — no SDL or rendering code
// lives here.  This keeps the loading path independent of the render backend.

#pragma once

#include "../core/Renderer.h"   // for Color, Rect

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clon3d {

// ---------------------------------------------------------------------------
// Attribute storage
// ---------------------------------------------------------------------------

/// A single XML attribute parsed from the theme file.
struct ThemeAttribute {
    std::string name;
    std::string value;
};

// ---------------------------------------------------------------------------
// ThemeElement
// ---------------------------------------------------------------------------

/// One node in the parsed theme tree.
///
/// Each ThemeElement corresponds to one XML element, e.g.:
///
///   <carousel name="systemCarousel">
///     <orientation>horizontal</orientation>
///     <pos x="0.0" y="0.3" />
///   </carousel>
///
/// The element type is stored in `type` (e.g. "carousel").
/// XML attributes on the element tag itself are in `attributes`.
/// Child text content (e.g. "horizontal" inside <orientation>) is in `text`.
/// Nested child elements are in `children`.
class ThemeElement {
public:
    using Ptr      = std::shared_ptr<ThemeElement>;
    using ChildVec = std::vector<Ptr>;

    // -------------------------------------------------------------------------
    // Identity
    // -------------------------------------------------------------------------

    std::string type;   ///< XML element tag name (e.g. "carousel", "image").
    std::string name;   ///< Value of the "name" attribute, if present.
    std::string text;   ///< Trimmed text content of this element.

    /// All XML attributes, keyed by attribute name.
    std::unordered_map<std::string, std::string> attributes;

    /// Child ThemeElements.
    ChildVec children;

    // -------------------------------------------------------------------------
    // Convenience accessors
    // -------------------------------------------------------------------------

    /// Returns the child element with the given tag, or nullptr if absent.
    Ptr child(const std::string& tag) const;

    /// Returns all direct children with the given tag.
    ChildVec childrenOf(const std::string& tag) const;

    // -------------------------------------------------------------------------
    // Typed attribute extraction
    // -------------------------------------------------------------------------

    /// Returns the attribute value as a string, or @p defaultVal.
    std::string attr(const std::string& key,
                     const std::string& defaultVal = {}) const;

    /// Returns the text content of the named child element, or @p defaultVal.
    std::string childText(const std::string& childTag,
                          const std::string& defaultVal = {}) const;

    float  childFloat(const std::string& childTag, float  defaultVal = 0.0f)  const;
    int    childInt  (const std::string& childTag, int    defaultVal = 0)     const;
    bool   childBool (const std::string& childTag, bool   defaultVal = false) const;

    /// Parse a <pos x="…" y="…"> child into a pair.
    std::pair<float,float> childPos(const std::string& childTag = "pos",
                                    float defaultX = 0.0f,
                                    float defaultY = 0.0f) const;

    /// Parse a <size w="…" h="…"> child into a pair.
    std::pair<float,float> childSize(const std::string& childTag = "size",
                                     float defaultW = 1.0f,
                                     float defaultH = 1.0f) const;

    /// Parse a <color r="…" g="…" b="…" a="…"> child into a Color.
    Color childColor(const std::string& childTag = "color",
                     Color defaultColor = Color::white()) const;
};

// ---------------------------------------------------------------------------
// Inline implementations
// ---------------------------------------------------------------------------

inline ThemeElement::Ptr ThemeElement::child(const std::string& tag) const {
    for (auto& c : children)
        if (c->type == tag) return c;
    return nullptr;
}

inline ThemeElement::ChildVec ThemeElement::childrenOf(const std::string& tag) const {
    ChildVec out;
    for (auto& c : children)
        if (c->type == tag) out.push_back(c);
    return out;
}

inline std::string ThemeElement::attr(const std::string& key,
                                       const std::string& defaultVal) const {
    auto it = attributes.find(key);
    return (it != attributes.end()) ? it->second : defaultVal;
}

inline std::string ThemeElement::childText(const std::string& childTag,
                                            const std::string& defaultVal) const {
    auto c = child(childTag);
    return c ? c->text : defaultVal;
}

inline float ThemeElement::childFloat(const std::string& childTag, float defaultVal) const {
    std::string t = childText(childTag);
    if (t.empty()) return defaultVal;
    try { return std::stof(t); } catch (...) { return defaultVal; }
}

inline int ThemeElement::childInt(const std::string& childTag, int defaultVal) const {
    std::string t = childText(childTag);
    if (t.empty()) return defaultVal;
    try { return std::stoi(t); } catch (...) { return defaultVal; }
}

inline bool ThemeElement::childBool(const std::string& childTag, bool defaultVal) const {
    std::string t = childText(childTag);
    if (t.empty()) return defaultVal;
    return (t == "true" || t == "1" || t == "yes");
}

inline std::pair<float,float> ThemeElement::childPos(const std::string& childTag,
                                                      float defaultX, float defaultY) const {
    auto c = child(childTag);
    if (!c) return {defaultX, defaultY};
    float x = defaultX, y = defaultY;
    try { x = std::stof(c->attr("x", std::to_string(defaultX))); } catch (...) {}
    try { y = std::stof(c->attr("y", std::to_string(defaultY))); } catch (...) {}
    return {x, y};
}

inline std::pair<float,float> ThemeElement::childSize(const std::string& childTag,
                                                       float defaultW, float defaultH) const {
    auto c = child(childTag);
    if (!c) return {defaultW, defaultH};
    float w = defaultW, h = defaultH;
    try { w = std::stof(c->attr("w", std::to_string(defaultW))); } catch (...) {}
    try { h = std::stof(c->attr("h", std::to_string(defaultH))); } catch (...) {}
    return {w, h};
}

inline Color ThemeElement::childColor(const std::string& childTag, Color defaultColor) const {
    auto c = child(childTag);
    if (!c) return defaultColor;
    Color col = defaultColor;
    try { col.r = static_cast<uint8_t>(std::stoi(c->attr("r", "255"))); } catch (...) {}
    try { col.g = static_cast<uint8_t>(std::stoi(c->attr("g", "255"))); } catch (...) {}
    try { col.b = static_cast<uint8_t>(std::stoi(c->attr("b", "255"))); } catch (...) {}
    try { col.a = static_cast<uint8_t>(std::stoi(c->attr("a", "255"))); } catch (...) {}
    return col;
}

} // namespace clon3d
