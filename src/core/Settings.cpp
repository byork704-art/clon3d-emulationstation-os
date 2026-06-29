// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — Settings.cpp

#include "Settings.h"
#include "Log.h"

#include <fstream>
#include <sstream>

namespace clon3d {

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

Settings& Settings::getInstance() {
    static Settings instance;
    return instance;
}

// ---------------------------------------------------------------------------
// Typed getters
// ---------------------------------------------------------------------------

bool Settings::getBool(const std::string& key, bool defaultVal) const {
    auto it = mValues.find(key);
    if (it == mValues.end()) return defaultVal;
    if (auto* v = std::get_if<bool>(&it->second)) return *v;
    return defaultVal;
}

int Settings::getInt(const std::string& key, int defaultVal) const {
    auto it = mValues.find(key);
    if (it == mValues.end()) return defaultVal;
    if (auto* v = std::get_if<int>(&it->second)) return *v;
    return defaultVal;
}

float Settings::getFloat(const std::string& key, float defaultVal) const {
    auto it = mValues.find(key);
    if (it == mValues.end()) return defaultVal;
    if (auto* v = std::get_if<float>(&it->second)) return *v;
    return defaultVal;
}

std::string Settings::getString(const std::string& key, const std::string& defaultVal) const {
    auto it = mValues.find(key);
    if (it == mValues.end()) return defaultVal;
    if (auto* v = std::get_if<std::string>(&it->second)) return *v;
    return defaultVal;
}

// ---------------------------------------------------------------------------
// Typed setters
// ---------------------------------------------------------------------------

void Settings::setBool  (const std::string& key, bool        value) { mValues[key] = value; }
void Settings::setInt   (const std::string& key, int         value) { mValues[key] = value; }
void Settings::setFloat (const std::string& key, float       value) { mValues[key] = value; }
void Settings::setString(const std::string& key, const std::string& value) { mValues[key] = value; }

// ---------------------------------------------------------------------------
// Persistence — minimal JSON-like serialisation (no external dependency)
// ---------------------------------------------------------------------------

/// Tiny helper: write a quoted, escaped JSON string.
static std::string jsonStr(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        switch (c) {
        case '"':  out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:   out += c;      break;
        }
    }
    out += '"';
    return out;
}

bool Settings::save(const std::string& filePath) const {
    std::ofstream f(filePath);
    if (!f) {
        LOG_ERROR("Settings") << "cannot open '" << filePath << "' for writing";
        return false;
    }

    f << "{\n";
    bool first = true;
    for (auto& [key, val] : mValues) {
        if (!first) f << ",\n";
        first = false;
        f << "  " << jsonStr(key) << ": ";
        std::visit([&](auto&& v) {
            using T = std::decay_t<decltype(v)>;
            if      constexpr (std::is_same_v<T, bool>)        f << (v ? "true" : "false");
            else if constexpr (std::is_same_v<T, int>)         f << v;
            else if constexpr (std::is_same_v<T, float>)       f << v;
            else if constexpr (std::is_same_v<T, std::string>) f << jsonStr(v);
        }, val);
    }
    f << "\n}\n";
    LOG_INFO("Settings") << "saved to '" << filePath << "'";
    return true;
}

/// Very small hand-rolled JSON loader — handles only the subset we write above.
/// For a production build with complex configs, swap in nlohmann/json or similar.
bool Settings::load(const std::string& filePath) {
    std::ifstream f(filePath);
    if (!f) {
        LOG_WARN("Settings") << "settings file not found: '" << filePath << "' — using defaults";
        return false;
    }

    // Accumulate all text and do a simple token parse.
    std::string src((std::istreambuf_iterator<char>(f)),
                     std::istreambuf_iterator<char>());

    // State machine: find key-value pairs.
    size_t i = 0;
    auto skip = [&]() {
        while (i < src.size() && (src[i] == ' ' || src[i] == '\t' ||
               src[i] == '\n' || src[i] == '\r' || src[i] == ',')) ++i;
    };
    auto readString = [&]() -> std::string {
        if (i >= src.size() || src[i] != '"') return {};
        ++i;
        std::string out;
        while (i < src.size() && src[i] != '"') {
            if (src[i] == '\\' && i + 1 < src.size()) {
                ++i;
                switch (src[i]) {
                case '"':  out += '"';  break;
                case '\\': out += '\\'; break;
                case 'n':  out += '\n'; break;
                case 'r':  out += '\r'; break;
                case 't':  out += '\t'; break;
                default:   out += src[i]; break;
                }
            } else {
                out += src[i];
            }
            ++i;
        }
        if (i < src.size()) ++i; // closing "
        return out;
    };

    skip();
    if (i < src.size() && src[i] == '{') ++i;

    while (i < src.size()) {
        skip();
        if (i >= src.size() || src[i] == '}') break;

        std::string key = readString();
        if (key.empty()) break;

        skip();
        if (i < src.size() && src[i] == ':') ++i;
        skip();

        // Determine value type by first character.
        if (i < src.size() && src[i] == '"') {
            setString(key, readString());
        } else if (src.substr(i, 4) == "true") {
            setBool(key, true);  i += 4;
        } else if (src.substr(i, 5) == "false") {
            setBool(key, false); i += 5;
        } else {
            // Number — detect float vs int by presence of '.'.
            size_t start = i;
            while (i < src.size() && (std::isdigit(static_cast<unsigned char>(src[i])) ||
                   src[i] == '-' || src[i] == '+' || src[i] == '.' || src[i] == 'e')) ++i;
            std::string numStr = src.substr(start, i - start);
            if (numStr.find('.') != std::string::npos || numStr.find('e') != std::string::npos) {
                try { setFloat(key, std::stof(numStr)); } catch (...) {}
            } else {
                try { setInt(key, std::stoi(numStr)); } catch (...) {}
            }
        }
    }

    LOG_INFO("Settings") << "loaded from '" << filePath << "'";
    return true;
}

} // namespace clon3d
