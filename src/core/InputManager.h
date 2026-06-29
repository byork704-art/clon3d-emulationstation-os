// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — InputManager.h
// Input abstraction: keyboard, gamepad, and joystick events.
//
// The engine uses a simple action-mapping layer.  Physical inputs are mapped
// to logical Actions.  Views receive Actions, not raw SDL keycodes.
//
// InputManager is a singleton accessed via InputManager::getInstance().

#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace clon3d {

// ---------------------------------------------------------------------------
// Logical input actions
// ---------------------------------------------------------------------------

/// High-level input action enum.
/// Raw hardware events are translated to these before reaching views.
enum class InputAction {
    None,

    // Navigation
    Up,
    Down,
    Left,
    Right,

    // Selection
    Accept,     ///< Confirm / select item (typically A button or Enter).
    Back,       ///< Go back / cancel (typically B button or Backspace).

    // System actions
    Start,      ///< Pause / system menu (typically Start button or Escape).
    Select,     ///< Secondary system action (typically Select button or F1).

    // Shoulder buttons
    PageUp,     ///< Scroll page up (typically L1 / PageUp).
    PageDown,   ///< Scroll page down (typically R1 / PageDown).

    // Debug/developer
    DebugToggle,
};

/// The state of an action at a given moment.
enum class InputState {
    Pressed,   ///< The button was pressed this frame.
    Released,  ///< The button was released this frame.
    Held,      ///< The button is being held down (auto-repeat fires).
};

/// A single input event delivered to views.
struct InputEvent {
    InputAction action{InputAction::None};
    InputState  state{InputState::Pressed};
};

// ---------------------------------------------------------------------------
// InputManager
// ---------------------------------------------------------------------------

/// Polls SDL events and translates them into InputEvents.
/// Registered listeners receive events in registration order.
class InputManager {
public:
    using Listener = std::function<bool(const InputEvent&)>;
    using ListenerHandle = size_t;

    static InputManager& getInstance();

    // -------------------------------------------------------------------------
    // Lifecycle
    // -------------------------------------------------------------------------

    /// Initialise SDL joystick/gamepad subsystems.
    bool init();

    /// Shut down and release all SDL input resources.
    void shutdown();

    // -------------------------------------------------------------------------
    // Per-frame processing
    // -------------------------------------------------------------------------

    /// Poll pending SDL events, translate them, and deliver to listeners.
    /// Call once per frame from the main loop.
    /// @returns false when the application should quit.
    bool processEvents();

    // -------------------------------------------------------------------------
    // Listener registration
    // -------------------------------------------------------------------------

    /// Register a listener that will receive translated InputEvents.
    /// Listeners are called in reverse registration order (last registered,
    /// first called) so that overlaying views can consume events before
    /// underlying ones.
    /// @returns An opaque handle that can be passed to removeListener().
    ListenerHandle addListener(Listener listener);

    /// Remove a previously registered listener.
    void removeListener(ListenerHandle handle);

    // -------------------------------------------------------------------------
    // Keyboard mapping query
    // -------------------------------------------------------------------------

    /// Return the InputAction mapped to an SDL scancode value (int to avoid
    /// pulling in SDL headers here).
    InputAction actionForScancode(int scancode) const;

private:
    InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void deliverEvent(const InputEvent& event);

    struct ListenerEntry {
        ListenerHandle handle;
        Listener       listener;
    };

    std::vector<ListenerEntry> mListeners;
    ListenerHandle             mNextHandle{1};
    bool                       mQuitRequested{false};
};

} // namespace clon3d
