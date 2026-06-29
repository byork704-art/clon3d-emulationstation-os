// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ThemeData.h
// Loads a theme from an XML file and exposes its parsed element tree.
//
// Theme XML structure
// -------------------
// A theme directory contains a `theme.xml` root file.  The root element is
// <theme> and may contain:
//
//   <formatVersion>2</formatVersion>    <!-- 1 = legacy RetroPie; 2 = Clon3D -->
//
//   <!-- System view layout (new Clon3D feature) -->
//   <systemView>
//     <carousel name="…"> … </carousel>
//     <image name="background"> … </image>
//     <overlay name="hud"> … </overlay>
//     <statusbar name="clock"> … </statusbar>
//     <navigation name="nav"> … </navigation>
//     <panel name="info"> … </panel>
//     <dashboard name="stats"> … </dashboard>
//   </systemView>
//
//   <!-- Game list view layout -->
//   <gamelistView> … </gamelistView>
//
//   <!-- Per-system overrides -->
//   <system name="nes">
//     <systemView> … </systemView>
//   </system>
//
// Backward compatibility
// ----------------------
// Themes with <formatVersion>1</formatVersion> (or no version tag) are treated
// as legacy themes.  In legacy mode the engine falls back to the original
// hardcoded carousel rendering.  None of the new tags are required.
//
// New vs legacy detection
// -----------------------
// A theme is considered "new" if formatVersion >= 2 OR if a <systemView>
// element containing a <carousel> child is present.

#pragma once

#include "ThemeElement.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace clon3d {

/// Parsed theme data.
///
/// Load with ThemeData::load(path).
/// Access the parsed tree with systemView(), gamelistView(), etc.
class ThemeData {
public:
    // -------------------------------------------------------------------------
    // Factory
    // -------------------------------------------------------------------------

    /// Load a theme from the given directory.
    /// Reads `<dir>/theme.xml` and any <include> files.
    /// @returns nullptr on failure (error is logged).
    static std::shared_ptr<ThemeData> load(const std::string& themeDir);

    // -------------------------------------------------------------------------
    // Version / compat
    // -------------------------------------------------------------------------

    /// Theme format version (1 = legacy, 2+ = Clon3D).
    int formatVersion() const { return mFormatVersion; }

    /// True when the theme uses the new Clon3D element syntax.
    /// When false the engine uses its own default rendering for everything.
    bool isNewFormat() const { return mFormatVersion >= 2; }

    /// True when the theme explicitly controls the system view carousel.
    /// This is the primary gate for the new carousel architecture.
    bool hasThemeControlledCarousel() const { return mHasThemeCarousel; }

    // -------------------------------------------------------------------------
    // Theme directory
    // -------------------------------------------------------------------------

    const std::string& themeDir() const { return mThemeDir; }

    /// Resolve a path that may be relative to the theme directory.
    std::string resolvePath(const std::string& path) const;

    // -------------------------------------------------------------------------
    // Root element accessors
    // -------------------------------------------------------------------------

    /// Returns the <systemView> element, or nullptr if not present.
    ThemeElement::Ptr systemView() const { return mSystemView; }

    /// Returns the <gamelistView> element, or nullptr if not present.
    ThemeElement::Ptr gamelistView() const { return mGamelistView; }

    /// Returns a per-system override <systemView> for the given system id,
    /// or nullptr if no override exists.
    ThemeElement::Ptr systemViewOverride(const std::string& systemId) const;

    /// Returns the raw root element of the parsed theme.
    ThemeElement::Ptr root() const { return mRoot; }

    // -------------------------------------------------------------------------
    // Convenience: read a named element from systemView
    // -------------------------------------------------------------------------

    /// Returns the carousel element from <systemView>, or nullptr.
    ThemeElement::Ptr carouselElement() const;

    /// Returns a named top-level child of <systemView> (e.g. "overlay").
    ThemeElement::Ptr systemViewElement(const std::string& type,
                                        const std::string& name = {}) const;

private:
    ThemeData() = default;

    // Parsing
    bool parseFile(const std::string& filePath);
    bool parseNode(ThemeElement::Ptr parent, const void* xmlNode);

    // Post-parse analysis
    void analyseCapabilities();

    // Data
    std::string       mThemeDir;
    ThemeElement::Ptr mRoot;
    ThemeElement::Ptr mSystemView;
    ThemeElement::Ptr mGamelistView;

    std::unordered_map<std::string, ThemeElement::Ptr> mSystemOverrides;

    int  mFormatVersion{1};
    bool mHasThemeCarousel{false};
};

} // namespace clon3d
