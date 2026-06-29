// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — TextComponent.h
// Renders a single line or wrapped block of text.
//
// Font rendering is handled through the engine's font subsystem (FreeType).
// If no font is set the component renders nothing but does not crash.

#pragma once

#include "IComponent.h"
#include <string>

namespace clon3d {

/// Horizontal alignment options.
enum class HAlign { Left, Centre, Right };

/// Vertical alignment options.
enum class VAlign { Top, Middle, Bottom };

/// Renders a text string inside its bounds.
class TextComponent : public IComponent {
public:
    explicit TextComponent(std::string text = {});

    // -------------------------------------------------------------------------
    // IComponent interface
    // -------------------------------------------------------------------------

    const char* typeName() const override { return "text"; }

    Rect bounds() const override { return mBounds; }
    void setBounds(const Rect& r) override { mBounds = r; }

    void update(float dt) override;
    void render(IRenderer& renderer) override;

    // -------------------------------------------------------------------------
    // Configuration
    // -------------------------------------------------------------------------

    void        setText  (const std::string& text)  { mText    = text;   }
    const std::string& text() const                 { return mText;       }

    void        setColor (Color c)                   { mColor   = c;      }
    Color       color()  const                       { return mColor;     }

    void        setFontSize(float size)              { mFontSize = size;  }
    float       fontSize()  const                    { return mFontSize;  }

    void        setHAlign(HAlign a)                  { mHAlign  = a;      }
    void        setVAlign(VAlign a)                  { mVAlign  = a;      }

    /// True: wrap text to multiple lines.  False: clip to one line.
    void        setWrap(bool wrap)                   { mWrap    = wrap;   }

private:
    Rect        mBounds;
    std::string mText;
    Color       mColor{Color::white()};
    float       mFontSize{0.05f};
    HAlign      mHAlign{HAlign::Centre};
    VAlign      mVAlign{VAlign::Middle};
    bool        mWrap{false};
};

} // namespace clon3d
