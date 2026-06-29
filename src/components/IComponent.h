// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — IComponent.h
// Base interface for all UI components.
//
// Every drawable entity in the engine (carousel, image, text, overlay…)
// implements IComponent.  The interface keeps rendering, updating, and
// input handling cleanly separated from one another.
//
// Lifecycle
// ---------
//   1. Component is constructed and configured (usually by ThemeComponentFactory).
//   2. update(dt) is called every frame to drive animations.
//   3. render(renderer) is called every frame to draw the component.
//   4. onInput(event) is called when user input arrives.  Returns true if the
//      event was consumed and should not propagate further.
//   5. Destructor releases any GPU/audio resources.

#pragma once

#include "../core/Renderer.h"
#include "../core/InputManager.h"
#include <memory>
#include <string>
#include <vector>

namespace clon3d {

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
class IComponent;
using ComponentPtr = std::shared_ptr<IComponent>;

// ---------------------------------------------------------------------------
// IComponent
// ---------------------------------------------------------------------------

/// Abstract base for every UI element.
class IComponent {
public:
    virtual ~IComponent() = default;

    // -------------------------------------------------------------------------
    // Identity
    // -------------------------------------------------------------------------

    /// Return the component type identifier (e.g. "carousel", "image").
    virtual const char* typeName() const = 0;

    /// Logical name given in the theme XML (may be empty).
    const std::string& name() const { return mName; }
    void               setName(const std::string& n) { mName = n; }

    // -------------------------------------------------------------------------
    // Geometry (normalised screen coords)
    // -------------------------------------------------------------------------

    virtual Rect     bounds() const = 0;
    virtual void     setBounds(const Rect& r) = 0;

    // -------------------------------------------------------------------------
    // Visibility
    // -------------------------------------------------------------------------

    bool isVisible() const { return mVisible; }
    void setVisible(bool v) { mVisible = v; }

    // -------------------------------------------------------------------------
    // Lifecycle callbacks
    // -------------------------------------------------------------------------

    /// Called once per frame.  @p dt is elapsed time in seconds.
    virtual void update(float dt) = 0;

    /// Render the component using the supplied renderer.
    virtual void render(IRenderer& renderer) = 0;

    /// Called with input events.
    /// @returns true if the event was consumed (stop propagation).
    virtual bool onInput(const InputEvent& event) { (void)event; return false; }

    // -------------------------------------------------------------------------
    // Children
    // -------------------------------------------------------------------------

    /// Add a child component that is updated/rendered after this one.
    void addChild(ComponentPtr child);

    /// Remove all children.
    void clearChildren();

    /// Access the ordered child list (read-only).
    const std::vector<ComponentPtr>& children() const { return mChildren; }

    /// Convenience: update then render all children.
    void updateChildren(float dt);
    void renderChildren(IRenderer& renderer);

protected:
    std::string mName;
    bool        mVisible{true};

private:
    std::vector<ComponentPtr> mChildren;
};

// ---------------------------------------------------------------------------
// Inline child management
// ---------------------------------------------------------------------------

inline void IComponent::addChild(ComponentPtr child) {
    mChildren.push_back(std::move(child));
}

inline void IComponent::clearChildren() {
    mChildren.clear();
}

inline void IComponent::updateChildren(float dt) {
    for (auto& c : mChildren)
        if (c->isVisible()) c->update(dt);
}

inline void IComponent::renderChildren(IRenderer& renderer) {
    for (auto& c : mChildren)
        if (c->isVisible()) c->render(renderer);
}

} // namespace clon3d
