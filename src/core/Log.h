// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — Log.h
// Lightweight logging macros used throughout the engine.
//
// Usage:
//   LOG_INFO("SystemView") << "initialising";
//   LOG_WARN("ThemeData") << "unknown element: " << name;
//   LOG_ERROR("Renderer") << "SDL_Init failed: " << SDL_GetError();

#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace clon3d {

/// Severity levels for the logging subsystem.
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error,
};

/// Internal helper that writes a single log line to stderr.
class LogLine {
public:
    LogLine(LogLevel level, const char* subsystem) {
        switch (level) {
        case LogLevel::Debug:   mStream << "[DBG]  "; break;
        case LogLevel::Info:    mStream << "[INFO] "; break;
        case LogLevel::Warning: mStream << "[WARN] "; break;
        case LogLevel::Error:   mStream << "[ERR]  "; break;
        }
        mStream << "[" << subsystem << "] ";
    }

    ~LogLine() {
        std::cerr << mStream.str() << "\n";
    }

    template<typename T>
    LogLine& operator<<(const T& v) {
        mStream << v;
        return *this;
    }

private:
    std::ostringstream mStream;
};

} // namespace clon3d

// Public logging macros — use these in all engine code.
#define LOG_DEBUG(subsystem) ::clon3d::LogLine(::clon3d::LogLevel::Debug,   subsystem)
#define LOG_INFO(subsystem)  ::clon3d::LogLine(::clon3d::LogLevel::Info,    subsystem)
#define LOG_WARN(subsystem)  ::clon3d::LogLine(::clon3d::LogLevel::Warning, subsystem)
#define LOG_ERROR(subsystem) ::clon3d::LogLine(::clon3d::LogLevel::Error,   subsystem)
