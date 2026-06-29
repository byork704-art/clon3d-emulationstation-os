// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — Renderer.h
// Abstract rendering interface.
//
// All engine code draws through this interface so that the backend
// (SDL2 + OpenGL ES on Raspberry Pi, or SDL2 + OpenGL on desktop) can be
// swapped without touching higher-level code.
//
// Coordinate system
// -----------------
// Normalised screen coordinates are used throughout the engine.
// (0,0) is the top-left corner; (1,1) is the bottom-right corner.
// The Renderer translates normalised coords to physical pixels internally.
//
// Colour format: 0xRRGGBBAA (big-endian bytes: R G B A).

#pragma once

#include <cstdint>
#include <string>

namespace clon3d {

/// RGBA colour value.  Bytes are (in memory order) R, G, B, A.
struct Color {
    uint8_t r{255}, g{255}, b{255}, a{255};

    static Color white()   { return {255, 255, 255, 255}; }
    static Color black()   { return {  0,   0,   0, 255}; }
    static Color transparent() { return {0, 0, 0, 0}; }

    uint32_t toRGBA() const {
        return (static_cast<uint32_t>(r) << 24) |
               (static_cast<uint32_t>(g) << 16) |
               (static_cast<uint32_t>(b) <<  8) |
               static_cast<uint32_t>(a);
    }
};

/// Axis-aligned rectangle in normalised screen space.
struct Rect {
    float x{0.0f}, y{0.0f}, w{0.0f}, h{0.0f};

    float right()  const { return x + w; }
    float bottom() const { return y + h; }

    bool contains(float px, float py) const {
        return px >= x && px <= right() && py >= y && py <= bottom();
    }
};

/// Opaque handle identifying a loaded GPU texture.
using TextureHandle = uint32_t;
static constexpr TextureHandle INVALID_TEXTURE = 0;

/// Abstract rendering backend.
/// Implementations: RendererSDL (desktop), RendererSDLGLES (Raspberry Pi).
class IRenderer {
public:
    virtual ~IRenderer() = default;

    // -------------------------------------------------------------------------
    // Lifecycle
    // -------------------------------------------------------------------------

    /// Initialise the render backend, create the window, and set up GL context.
    /// @param title    Window title.
    /// @param width    Window width in pixels (ignored if fullscreen).
    /// @param height   Window height in pixels (ignored if fullscreen).
    /// @param fullscreen  Create a full-screen window.
    /// @returns true on success.
    virtual bool init(const std::string& title, int width, int height, bool fullscreen) = 0;

    /// Shut down the render backend and release all GPU resources.
    virtual void shutdown() = 0;

    /// Begin a new frame: clear the screen to @p clearColor.
    virtual void beginFrame(Color clearColor = Color::black()) = 0;

    /// End the current frame and flip the display buffer.
    virtual void endFrame() = 0;

    // -------------------------------------------------------------------------
    // Texture management
    // -------------------------------------------------------------------------

    /// Load a texture from a PNG/JPEG file.
    /// @returns A valid TextureHandle, or INVALID_TEXTURE on failure.
    virtual TextureHandle loadTexture(const std::string& filePath) = 0;

    /// Release a previously loaded texture.
    virtual void freeTexture(TextureHandle handle) = 0;

    // -------------------------------------------------------------------------
    // Drawing primitives (normalised coordinates)
    // -------------------------------------------------------------------------

    /// Draw a filled rectangle.
    virtual void drawRect(const Rect& rect, Color color) = 0;

    /// Draw a textured rectangle.
    /// If @p tint is not white the texture pixels are multiplied by the tint.
    virtual void drawTexture(TextureHandle handle, const Rect& dest,
                             Color tint = Color::white()) = 0;

    /// Draw a textured rectangle with a sub-region of the source texture.
    /// @p src is in normalised texture coordinates (0–1).
    virtual void drawTextureCrop(TextureHandle handle,
                                 const Rect& src, const Rect& dest,
                                 Color tint = Color::white()) = 0;

    // -------------------------------------------------------------------------
    // Clipping
    // -------------------------------------------------------------------------

    /// Set the clip rectangle; nothing outside this region will be drawn.
    virtual void setClipRect(const Rect& rect) = 0;

    /// Remove any active clip rectangle.
    virtual void clearClipRect() = 0;

    // -------------------------------------------------------------------------
    // Query
    // -------------------------------------------------------------------------

    /// Physical screen width in pixels.
    virtual int screenWidth()  const = 0;
    /// Physical screen height in pixels.
    virtual int screenHeight() const = 0;
};

} // namespace clon3d
