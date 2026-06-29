// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — ViewController.h
// Top-level view manager.
//
// ViewController owns all views (SystemView, GamelistView, …) and manages
// transitions between them.  It owns the main update/render loop, receives
// input from InputManager, and delegates to the active view.
//
// State machine
// -------------
//   SystemView  ←→  GamelistView  (←→ future: GameView, SettingsView, …)
//
// The controller pushes/pops views onto a stack so that Back always returns
// to the previous view.

#pragma once

#include "../components/IComponent.h"

#include <functional>
#include <memory>
#include <stack>
#include <string>

namespace clon3d {

class IRenderer;
class SystemView;
class ThemeData;

/// Manages all top-level views and transitions between them.
class ViewController {
public:
    ViewController();
    ~ViewController() = default;

    // -------------------------------------------------------------------------
    // Initialisation
    // -------------------------------------------------------------------------

    /// Set the renderer used by all views.
    void setRenderer(IRenderer* renderer) { mRenderer = renderer; }

    /// Load the system list from `es_systems.cfg` and initialise SystemView.
    /// @param systemsConfigPath  Path to the es_systems.cfg XML file.
    /// @param theme              Active theme (may be nullptr for legacy mode).
    bool initialise(const std::string& systemsConfigPath,
                    std::shared_ptr<ThemeData> theme);

    // -------------------------------------------------------------------------
    // Per-frame callbacks (called from the main loop)
    // -------------------------------------------------------------------------

    /// Update the active view.  @p dt is elapsed time in seconds.
    void update(float dt);

    /// Render the active view.
    void render();

    // -------------------------------------------------------------------------
    // Input
    // -------------------------------------------------------------------------

    /// Deliver an input event to the active view.
    bool onInput(const InputEvent& event);

    // -------------------------------------------------------------------------
    // Navigation
    // -------------------------------------------------------------------------

    /// Push a new view onto the stack (shown immediately).
    void pushView(std::shared_ptr<IComponent> view);

    /// Pop the current view and return to the previous one.
    void popView();

    /// Direct access to the SystemView.
    std::shared_ptr<SystemView> systemView() const { return mSystemView; }

private:
    IRenderer*                    mRenderer{nullptr};
    std::shared_ptr<SystemView>   mSystemView;
    std::stack<std::shared_ptr<IComponent>> mViewStack;
};

} // namespace clon3d
