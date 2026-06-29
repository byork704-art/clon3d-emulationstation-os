// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — main.cpp
// Application entry point.
//
// Start-up sequence
// -----------------
//   1. Load settings from ~/.config/clon3d-es/settings.json
//   2. Initialise SDL2 and create the render window
//   3. Initialise the input manager
//   4. Load the active theme
//   5. Initialise the ViewController with es_systems.cfg
//   6. Run the main loop (process input → update → render)
//   7. Shutdown and exit

#include "core/Settings.h"
#include "core/Log.h"
#include "core/InputManager.h"
#include "theme/ThemeData.h"
#include "views/ViewController.h"

// SDL2 is only directly used here for window / renderer creation.
// Everything else goes through the IRenderer abstraction.
#include <SDL2/SDL.h>

#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Minimal SDL2 renderer implementation (stub for compilation)
// ---------------------------------------------------------------------------
// A complete IRenderer implementation backed by SDL2+OpenGL lives in
// src/core/RendererSDL.cpp.  That file is omitted from Stage 1 to keep the
// build straightforward.  The stub below lets the project compile and the
// architecture to be demonstrated without a GPU context.

#include "core/Renderer.h"

namespace clon3d {

class StubRenderer : public IRenderer {
public:
    bool init(const std::string& title, int width, int height, bool fullscreen) override {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            LOG_ERROR("StubRenderer") << "SDL_Init failed: " << SDL_GetError();
            return false;
        }
        Uint32 flags = SDL_WINDOW_SHOWN;
        if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        mWindow = SDL_CreateWindow(title.c_str(),
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   width, height, flags);
        if (!mWindow) {
            LOG_ERROR("StubRenderer") << "SDL_CreateWindow failed: " << SDL_GetError();
            return false;
        }
        mSDLRenderer = SDL_CreateRenderer(mWindow, -1,
                           SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!mSDLRenderer) {
            LOG_ERROR("StubRenderer") << "SDL_CreateRenderer failed: " << SDL_GetError();
            return false;
        }
        SDL_GetWindowSize(mWindow, &mWidth, &mHeight);
        LOG_INFO("StubRenderer") << "window " << mWidth << "×" << mHeight;
        return true;
    }

    void shutdown() override {
        if (mSDLRenderer) { SDL_DestroyRenderer(mSDLRenderer); mSDLRenderer = nullptr; }
        if (mWindow)      { SDL_DestroyWindow(mWindow);        mWindow      = nullptr; }
        SDL_Quit();
    }

    void beginFrame(Color c) override {
        SDL_SetRenderDrawColor(mSDLRenderer, c.r, c.g, c.b, c.a);
        SDL_RenderClear(mSDLRenderer);
    }

    void endFrame() override {
        SDL_RenderPresent(mSDLRenderer);
    }

    TextureHandle loadTexture(const std::string& filePath) override {
        (void)filePath;
        LOG_DEBUG("StubRenderer") << "loadTexture: " << filePath << " (stub)";
        return INVALID_TEXTURE;
    }
    void freeTexture(TextureHandle) override {}

    void drawRect(const Rect& rect, Color color) override {
        SDL_SetRenderDrawColor(mSDLRenderer, color.r, color.g, color.b, color.a);
        SDL_SetRenderDrawBlendMode(mSDLRenderer, SDL_BLENDMODE_BLEND);
        SDL_Rect r = toSDL(rect);
        SDL_RenderFillRect(mSDLRenderer, &r);
    }

    void drawTexture(TextureHandle, const Rect&, Color) override {}
    void drawTextureCrop(TextureHandle, const Rect&, const Rect&, Color) override {}

    void setClipRect(const Rect& rect) override {
        SDL_Rect r = toSDL(rect);
        SDL_RenderSetClipRect(mSDLRenderer, &r);
    }
    void clearClipRect() override {
        SDL_RenderSetClipRect(mSDLRenderer, nullptr);
    }

    int screenWidth()  const override { return mWidth;  }
    int screenHeight() const override { return mHeight; }

private:
    SDL_Rect toSDL(const Rect& r) const {
        return {
            static_cast<int>(r.x * mWidth),
            static_cast<int>(r.y * mHeight),
            static_cast<int>(r.w * mWidth),
            static_cast<int>(r.h * mHeight)
        };
    }

    SDL_Window*   mWindow{nullptr};
    SDL_Renderer* mSDLRenderer{nullptr};
    int           mWidth{1280}, mHeight{720};
};

} // namespace clon3d

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string findConfigFile(const std::string& name) {
    // Search order: current directory, ~/.config/clon3d-es/, /etc/clon3d-es/
    std::vector<std::string> candidates = {
        "./" + name,
        std::string(getenv("HOME") ? getenv("HOME") : "") + "/.config/clon3d-es/" + name,
        "/etc/clon3d-es/" + name,
    };
    for (auto& p : candidates)
        if (fs::exists(p)) return p;
    return {};
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    using namespace clon3d;

    // ---- 1. Load settings --------------------------------------------------
    auto& settings = Settings::getInstance();
    std::string settingsPath = findConfigFile("settings.json");
    if (!settingsPath.empty()) settings.load(settingsPath);

    bool debug = settings.isDebugMode();
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--debug") settings.setBool("debug", true);
        if (std::string(argv[i]) == "--windowed") settings.setBool("fullscreen", false);
    }

    LOG_INFO("main") << "Clon3D EmulationStation starting up";
    if (debug) LOG_DEBUG("main") << "debug mode enabled";

    // ---- 2. Initialise renderer --------------------------------------------
    StubRenderer renderer;
    if (!renderer.init("Clon3D EmulationStation",
                       settings.screenWidth(),
                       settings.screenHeight(),
                       settings.isFullscreen())) {
        return 1;
    }

    // ---- 3. Initialise input manager ----------------------------------------
    auto& input = InputManager::getInstance();
    if (!input.init()) {
        LOG_WARN("main") << "input manager init failed — keyboard only";
    }

    // ---- 4. Load theme -------------------------------------------------------
    std::shared_ptr<ThemeData> theme;
    std::string themeDir = settings.themePath();
    if (fs::exists(themeDir + "/theme.xml")) {
        theme = ThemeData::load(themeDir);
        if (!theme)
            LOG_WARN("main") << "theme failed to load — using legacy mode";
    } else {
        LOG_INFO("main") << "no theme.xml at '" << themeDir << "' — legacy mode";
    }

    // ---- 5. Initialise ViewController ---------------------------------------
    ViewController viewController;
    viewController.setRenderer(&renderer);

    std::string systemsCfg = findConfigFile("es_systems.cfg");
    if (systemsCfg.empty()) {
        // Create a minimal fallback systems list so the UI is still usable.
        systemsCfg = "/tmp/clon3d_systems_demo.cfg";
        if (!fs::exists(systemsCfg)) {
            std::ofstream f(systemsCfg);
            f << "<?xml version=\"1.0\"?>\n<systemList>\n"
              << "  <system><name>nes</name><fullname>Nintendo Entertainment System</fullname></system>\n"
              << "  <system><name>snes</name><fullname>Super Nintendo</fullname></system>\n"
              << "  <system><name>megadrive</name><fullname>Sega Mega Drive</fullname></system>\n"
              << "  <system><name>psx</name><fullname>Sony PlayStation</fullname></system>\n"
              << "  <system><name>n64</name><fullname>Nintendo 64</fullname></system>\n"
              << "</systemList>\n";
        }
    }

    if (!viewController.initialise(systemsCfg, theme)) {
        LOG_ERROR("main") << "ViewController initialisation failed";
        renderer.shutdown();
        return 1;
    }

    // Register input listener — forward events to the view controller.
    input.addListener([&viewController](const InputEvent& ev) {
        return viewController.onInput(ev);
    });

    // ---- 6. Main loop -------------------------------------------------------
    LOG_INFO("main") << "entering main loop";

    uint64_t lastTime = SDL_GetPerformanceCounter();
    bool running = true;

    while (running) {
        // Process input.
        running = input.processEvents();

        // Compute delta time.
        uint64_t now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(now - lastTime) /
                   static_cast<float>(SDL_GetPerformanceFrequency());
        lastTime = now;
        dt = std::min(dt, 0.1f); // cap to avoid huge jumps after pauses

        // Update.
        viewController.update(dt);

        // Render.
        renderer.beginFrame(Color::black());
        viewController.render();
        renderer.endFrame();
    }

    // ---- 7. Shutdown --------------------------------------------------------
    LOG_INFO("main") << "shutting down";
    input.shutdown();
    renderer.shutdown();

    return 0;
}
