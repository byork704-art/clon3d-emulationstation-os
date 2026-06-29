// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ThemeData.cpp
//
// XML parsing uses pugixml (header-only mode).

#include "ThemeData.h"
#include "../core/Log.h"

#include <pugixml.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace clon3d {

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------

std::shared_ptr<ThemeData> ThemeData::load(const std::string& themeDir) {
    auto theme = std::shared_ptr<ThemeData>(new ThemeData());
    theme->mThemeDir = themeDir;

    std::string themePath = themeDir + "/theme.xml";
    if (!theme->parseFile(themePath)) {
        LOG_ERROR("ThemeData") << "failed to load theme from: " << themePath;
        return nullptr;
    }

    theme->analyseCapabilities();
    LOG_INFO("ThemeData") << "loaded theme v" << theme->mFormatVersion
                          << " from: " << themeDir
                          << (theme->mHasThemeCarousel ? " [theme-controlled carousel]" : " [legacy carousel]");
    return theme;
}

// ---------------------------------------------------------------------------
// Path resolution
// ---------------------------------------------------------------------------

std::string ThemeData::resolvePath(const std::string& path) const {
    if (path.empty()) return path;
    // Absolute paths are returned unchanged.
    if (path[0] == '/') return path;
    // Paths starting with "./" or "../" are relative to the theme directory.
    return mThemeDir + "/" + path;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

ThemeElement::Ptr ThemeData::systemViewOverride(const std::string& systemId) const {
    auto it = mSystemOverrides.find(systemId);
    return (it != mSystemOverrides.end()) ? it->second : nullptr;
}

ThemeElement::Ptr ThemeData::carouselElement() const {
    if (!mSystemView) return nullptr;
    return mSystemView->child("carousel");
}

ThemeElement::Ptr ThemeData::systemViewElement(const std::string& type,
                                                const std::string& name) const {
    if (!mSystemView) return nullptr;
    for (auto& c : mSystemView->children) {
        if (c->type == type) {
            if (name.empty() || c->name == name) return c;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// XML parsing (pugixml)
// ---------------------------------------------------------------------------

static std::string trimString(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return {};
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

bool ThemeData::parseFile(const std::string& filePath) {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filePath.c_str());
    if (!result) {
        LOG_ERROR("ThemeData") << "XML parse error in '" << filePath
                               << "': " << result.description();
        return false;
    }

    mRoot = std::make_shared<ThemeElement>();
    mRoot->type = "root";

    pugi::xml_node themeNode = doc.child("theme");
    if (!themeNode) {
        LOG_WARN("ThemeData") << "no <theme> root element found in '" << filePath << "'";
        return false;
    }

    // Collect attributes of <theme> element itself.
    for (auto attr : themeNode.attributes())
        mRoot->attributes[attr.name()] = attr.value();

    // Parse child elements.
    for (auto child : themeNode.children()) {
        if (child.type() != pugi::node_element) continue;

        auto elem = std::make_shared<ThemeElement>();
        elem->type = child.name();

        // Handle <include> — load the referenced file and graft its children.
        if (elem->type == "include") {
            std::string incPath = resolvePath(trimString(child.child_value()));
            LOG_DEBUG("ThemeData") << "including: " << incPath;
            // Inline include: re-parse and add children directly to root.
            pugi::xml_document incDoc;
            if (incDoc.load_file(incPath.c_str())) {
                pugi::xml_node incRoot = incDoc.child("theme");
                if (!incRoot) incRoot = incDoc.first_child();
                for (auto incChild : incRoot.children()) {
                    if (incChild.type() != pugi::node_element) continue;
                    auto incElem = std::make_shared<ThemeElement>();
                    parseNode(incElem, &incChild);
                    mRoot->children.push_back(incElem);
                }
            } else {
                LOG_WARN("ThemeData") << "failed to include: " << incPath;
            }
            continue;
        }

        parseNode(elem, &child);
        mRoot->children.push_back(elem);
    }

    return true;
}

bool ThemeData::parseNode(ThemeElement::Ptr elem, const void* vnode) {
    const pugi::xml_node* node = static_cast<const pugi::xml_node*>(vnode);

    elem->type = node->name();

    // Collect XML attributes.
    for (auto attr : node->attributes())
        elem->attributes[attr.name()] = attr.value();

    // Convenience: pull "name" attribute into the name field.
    auto nameIt = elem->attributes.find("name");
    if (nameIt != elem->attributes.end())
        elem->name = nameIt->second;

    // Collect trimmed text content (if this is a leaf node).
    if (!node->first_child() || node->first_child().type() == pugi::node_pcdata) {
        elem->text = trimString(node->child_value());
    }

    // Recursively parse children.
    for (auto child : node->children()) {
        if (child.type() != pugi::node_element) continue;
        auto childElem = std::make_shared<ThemeElement>();
        parseNode(childElem, &child);
        elem->children.push_back(childElem);
    }

    return true;
}

// ---------------------------------------------------------------------------
// Post-parse capability analysis
// ---------------------------------------------------------------------------

void ThemeData::analyseCapabilities() {
    if (!mRoot) return;

    // Extract format version.
    for (auto& c : mRoot->children) {
        if (c->type == "formatVersion") {
            try { mFormatVersion = std::stoi(c->text); } catch (...) {}
            break;
        }
    }

    // Extract top-level systemView and gamelistView.
    for (auto& c : mRoot->children) {
        if (c->type == "systemView" && !mSystemView)
            mSystemView = c;
        else if (c->type == "gamelistView" && !mGamelistView)
            mGamelistView = c;
        else if (c->type == "system") {
            // Per-system override block.
            std::string sysName = c->name;
            if (!sysName.empty()) {
                auto sv = c->child("systemView");
                if (sv) mSystemOverrides[sysName] = sv;
            }
        }
    }

    // Detect theme-controlled carousel.
    if (mSystemView && mSystemView->child("carousel"))
        mHasThemeCarousel = true;

    // If formatVersion >= 2 and systemView present, consider it new format.
    if (mFormatVersion >= 2 && mSystemView)
        mHasThemeCarousel = true;
}

} // namespace clon3d
