// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ViewController.cpp

#include "ViewController.h"
#include "SystemView.h"
#include "../core/Log.h"

#include <pugixml.hpp>

namespace clon3d {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

ViewController::ViewController() = default;

// ---------------------------------------------------------------------------
// Helpers: parse es_systems.cfg
// ---------------------------------------------------------------------------

static std::vector<SystemDescriptor> parseSystemsCfg(const std::string& path) {
    std::vector<SystemDescriptor> systems;

    pugi::xml_document doc;
    if (!doc.load_file(path.c_str())) {
        LOG_ERROR("ViewController") << "cannot open systems config: " << path;
        return systems;
    }

    pugi::xml_node root = doc.child("systemList");
    if (!root) root = doc.first_child();

    for (auto sys : root.children("system")) {
        SystemDescriptor desc;
        desc.id   = sys.child_value("name");
        desc.name = sys.child_value("fullname");
        if (desc.name.empty()) desc.name = desc.id;

        // Optional: logo path attribute (not in vanilla es_systems.cfg but
        // theme directories may extend it).
        desc.logoPath = sys.child_value("logo");

        systems.push_back(std::move(desc));
    }

    LOG_INFO("ViewController") << "parsed " << systems.size()
                               << " system(s) from " << path;
    return systems;
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

bool ViewController::initialise(const std::string& systemsConfigPath,
                                 std::shared_ptr<ThemeData> theme) {
    auto systems = parseSystemsCfg(systemsConfigPath);

    mSystemView = std::make_shared<SystemView>();
    mSystemView->setBounds({0.0f, 0.0f, 1.0f, 1.0f});
    mSystemView->setSystems(std::move(systems));

    if (theme && mRenderer) {
        mSystemView->setTheme(theme, *mRenderer);
    }

    // Wire system-selected callback to launch gamelist view.
    mSystemView->setOnSystemSelected([this](const SystemDescriptor& sys) {
        LOG_INFO("ViewController") << "system selected: " << sys.name;
        // TODO: push GamelistView for the selected system.
    });

    // Seed the view stack with SystemView.
    while (!mViewStack.empty()) mViewStack.pop();
    mViewStack.push(mSystemView);

    return true;
}

// ---------------------------------------------------------------------------
// Per-frame
// ---------------------------------------------------------------------------

void ViewController::update(float dt) {
    if (!mViewStack.empty())
        mViewStack.top()->update(dt);
}

void ViewController::render() {
    if (!mViewStack.empty() && mRenderer)
        mViewStack.top()->render(*mRenderer);
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------

bool ViewController::onInput(const InputEvent& event) {
    if (mViewStack.empty()) return false;
    return mViewStack.top()->onInput(event);
}

// ---------------------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------------------

void ViewController::pushView(std::shared_ptr<IComponent> view) {
    mViewStack.push(std::move(view));
}

void ViewController::popView() {
    if (mViewStack.size() > 1)
        mViewStack.pop();
}

} // namespace clon3d
