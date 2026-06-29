// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — InputManager.cpp

#include "InputManager.h"
#include "Log.h"

// SDL2 headers — only included in the .cpp to keep the public header clean.
#include <SDL2/SDL.h>

namespace clon3d {

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool InputManager::init() {
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0) {
        LOG_ERROR("InputManager") << "SDL gamepad init failed: " << SDL_GetError();
        return false;
    }

    // Open all connected joysticks/gamepads at startup.
    int count = SDL_NumJoysticks();
    LOG_INFO("InputManager") << "found " << count << " joystick(s)";
    for (int i = 0; i < count; ++i) {
        if (SDL_IsGameController(i)) {
            SDL_GameControllerOpen(i);
        } else {
            SDL_JoystickOpen(i);
        }
    }
    return true;
}

void InputManager::shutdown() {
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
    LOG_INFO("InputManager") << "shutdown";
}

// ---------------------------------------------------------------------------
// Keyboard mapping
// ---------------------------------------------------------------------------

InputAction InputManager::actionForScancode(int scancode) const {
    // Maps SDL_Scancode values to logical actions.
    // Physical gamepad buttons are handled separately via SDL controller events.
    switch (static_cast<SDL_Scancode>(scancode)) {
    case SDL_SCANCODE_UP:       return InputAction::Up;
    case SDL_SCANCODE_DOWN:     return InputAction::Down;
    case SDL_SCANCODE_LEFT:     return InputAction::Left;
    case SDL_SCANCODE_RIGHT:    return InputAction::Right;
    case SDL_SCANCODE_RETURN:
    case SDL_SCANCODE_KP_ENTER: return InputAction::Accept;
    case SDL_SCANCODE_BACKSPACE:return InputAction::Back;
    case SDL_SCANCODE_ESCAPE:   return InputAction::Start;
    case SDL_SCANCODE_F1:       return InputAction::Select;
    case SDL_SCANCODE_PAGEUP:   return InputAction::PageUp;
    case SDL_SCANCODE_PAGEDOWN: return InputAction::PageDown;
    case SDL_SCANCODE_F12:      return InputAction::DebugToggle;
    default:                    return InputAction::None;
    }
}

// ---------------------------------------------------------------------------
// Event delivery
// ---------------------------------------------------------------------------

void InputManager::deliverEvent(const InputEvent& event) {
    if (event.action == InputAction::None) return;

    // Walk in reverse so the topmost view processes first.
    for (auto it = mListeners.rbegin(); it != mListeners.rend(); ++it) {
        if (it->listener(event)) return; // consumed
    }
}

// ---------------------------------------------------------------------------
// Per-frame polling
// ---------------------------------------------------------------------------

bool InputManager::processEvents() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {

        case SDL_QUIT:
            mQuitRequested = true;
            break;

        // ------------------------------------------------------------------
        // Keyboard
        // ------------------------------------------------------------------
        case SDL_KEYDOWN: {
            auto action = actionForScancode(sdlEvent.key.keysym.scancode);
            bool repeat = sdlEvent.key.repeat != 0;
            deliverEvent({action, repeat ? InputState::Held : InputState::Pressed});
            break;
        }
        case SDL_KEYUP: {
            auto action = actionForScancode(sdlEvent.key.keysym.scancode);
            deliverEvent({action, InputState::Released});
            break;
        }

        // ------------------------------------------------------------------
        // Gamepad (SDL game controller API)
        // ------------------------------------------------------------------
        case SDL_CONTROLLERBUTTONDOWN: {
            InputAction action = InputAction::None;
            switch (sdlEvent.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:    action = InputAction::Up;       break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  action = InputAction::Down;     break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  action = InputAction::Left;     break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: action = InputAction::Right;    break;
            case SDL_CONTROLLER_BUTTON_A:          action = InputAction::Accept;   break;
            case SDL_CONTROLLER_BUTTON_B:          action = InputAction::Back;     break;
            case SDL_CONTROLLER_BUTTON_START:      action = InputAction::Start;    break;
            case SDL_CONTROLLER_BUTTON_BACK:       action = InputAction::Select;   break;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  action = InputAction::PageUp;   break;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: action = InputAction::PageDown; break;
            default: break;
            }
            deliverEvent({action, InputState::Pressed});
            break;
        }
        case SDL_CONTROLLERBUTTONUP: {
            InputAction action = InputAction::None;
            switch (sdlEvent.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:    action = InputAction::Up;       break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  action = InputAction::Down;     break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  action = InputAction::Left;     break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: action = InputAction::Right;    break;
            case SDL_CONTROLLER_BUTTON_A:          action = InputAction::Accept;   break;
            case SDL_CONTROLLER_BUTTON_B:          action = InputAction::Back;     break;
            case SDL_CONTROLLER_BUTTON_START:      action = InputAction::Start;    break;
            case SDL_CONTROLLER_BUTTON_BACK:       action = InputAction::Select;   break;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  action = InputAction::PageUp;   break;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: action = InputAction::PageDown; break;
            default: break;
            }
            deliverEvent({action, InputState::Released});
            break;
        }

        // ------------------------------------------------------------------
        // Controller connect / disconnect
        // ------------------------------------------------------------------
        case SDL_CONTROLLERDEVICEADDED:
            SDL_GameControllerOpen(sdlEvent.cdevice.which);
            LOG_INFO("InputManager") << "controller connected (index "
                                     << sdlEvent.cdevice.which << ")";
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            LOG_INFO("InputManager") << "controller disconnected";
            break;

        default:
            break;
        }
    }

    return !mQuitRequested;
}

// ---------------------------------------------------------------------------
// Listener management
// ---------------------------------------------------------------------------

InputManager::ListenerHandle InputManager::addListener(Listener listener) {
    ListenerHandle h = mNextHandle++;
    mListeners.push_back({h, std::move(listener)});
    return h;
}

void InputManager::removeListener(ListenerHandle handle) {
    mListeners.erase(
        std::remove_if(mListeners.begin(), mListeners.end(),
            [handle](const ListenerEntry& e) { return e.handle == handle; }),
        mListeners.end());
}

} // namespace clon3d
