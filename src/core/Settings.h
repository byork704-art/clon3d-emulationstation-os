// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — Settings.h
// Runtime-configurable key/value store used throughout the engine.
//
// Settings is a singleton accessed via Settings::getInstance().
// Values are typed: bool, int, float, and string are supported.
// Persistent settings are written to/from a JSON file at startup/shutdown.

#pragma once

#include <string>
#include <unordered_map>
#include <variant>

namespace clon3d {

/// Supported setting value types.
using SettingValue = std::variant<bool, int, float, std::string>;

/// Global settings singleton.
/// All engine subsystems may read from this object.
/// Settings are loaded once at startup and saved on exit.
class Settings {
public:
    /// Returns the global singleton instance.
    static Settings& getInstance();

    // -------------------------------------------------------------------------
    // Typed accessors
    // -------------------------------------------------------------------------

    bool        getBool  (const std::string& key, bool        defaultVal = false)  const;
    int         getInt   (const std::string& key, int         defaultVal = 0)      const;
    float       getFloat (const std::string& key, float       defaultVal = 0.0f)   const;
    std::string getString(const std::string& key, const std::string& defaultVal = "") const;

    void setBool  (const std::string& key, bool        value);
    void setInt   (const std::string& key, int         value);
    void setFloat (const std::string& key, float       value);
    void setString(const std::string& key, const std::string& value);

    // -------------------------------------------------------------------------
    // Persistence
    // -------------------------------------------------------------------------

    /// Load settings from the given JSON file path.
    /// Missing keys are silently ignored; the engine uses defaults instead.
    bool load(const std::string& filePath);

    /// Save all in-memory settings to the given JSON file path.
    bool save(const std::string& filePath) const;

    // -------------------------------------------------------------------------
    // Convenience helpers
    // -------------------------------------------------------------------------

    /// True when the engine should log verbose output.
    bool isDebugMode() const { return getBool("debug", false); }

    /// Screen resolution helpers.
    int screenWidth()  const { return getInt("screenWidth",  1280); }
    int screenHeight() const { return getInt("screenHeight", 720);  }

    /// True when the display should be full-screen.
    bool isFullscreen() const { return getBool("fullscreen", true); }

    /// Path to the active theme directory (absolute or relative to ES root).
    std::string themePath() const { return getString("themePath", "themes/clon3d-default"); }

private:
    Settings() = default;
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;

    std::unordered_map<std::string, SettingValue> mValues;
};

} // namespace clon3d
