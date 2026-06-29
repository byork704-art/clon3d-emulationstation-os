#pragma once
#ifndef CLON3D_VIEWS_DASHBOARD_VIEW_H
#define CLON3D_VIEWS_DASHBOARD_VIEW_H

// Clon3D Cyberdeck OS — Dashboard System Selection View
//
// DashboardView replaces the stock EmulationStation SystemView (carousel) with
// a fullscreen dashboard-style system selection experience.
//
// Layout (normalised coordinates, origin = top-left):
//
//   ┌──────────────────────────────────────────────────────────────────────┐
//   │  Background layer  (full screen)                                     │
//   │  ┌──────────────────────────┐  ┌───────────────────────────────────┐ │
//   │  │  Art Panel  (55 %)       │  │  Info Panel  (45 %)               │ │
//   │  │                          │  │                                   │ │
//   │  │   Giant system artwork   │  │   System full name  (large)       │ │
//   │  │         / logo           │  │   Game count                      │ │
//   │  │                          │  │   Platform hint                   │ │
//   │  │                          │  │                                   │ │
//   │  └──────────────────────────┘  └───────────────────────────────────┘ │
//   │  ┌──────────────────────────────────────────────────────────────────┐ │
//   │  │  HUD bar  (bottom 7 %)   —  clock | nav | system count          │ │
//   │  └──────────────────────────────────────────────────────────────────┘ │
//   └──────────────────────────────────────────────────────────────────────┘
//
// Enabling dashboard mode:
//   Set the "DashboardMode" boolean in es_settings.xml to true, or use the
//   in-game UI Settings menu (added in a later phase).
//
// Legacy fallback:
//   When "DashboardMode" is false (default), ViewController continues to
//   instantiate the original SystemView carousel.  Both code paths are fully
//   independent.
//
// See docs/RENDERING_ARCHITECTURE.md for the full architecture description.

#include "GuiComponent.h"
#include "HelpStyle.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "resources/Font.h"
#include <memory>
#include <vector>

class SystemData;
class HudOverlay;

class DashboardView : public GuiComponent
{
public:
    explicit DashboardView(Window* window);
    ~DashboardView() override;

    // Move the cursor to the given system (optionally animating the transition).
    void goToSystem(SystemData* system, bool animate);

    // Return the currently selected system.
    SystemData* getSelected() const;

    // ── GuiComponent interface ────────────────────────────────────────────

    bool  input(InputConfig* config, Input input) override;
    void  update(int deltaTime) override;
    void  render(const Transform4x4f& parentTrans) override;

    void  onShow() override;
    void  onHide() override;

    std::vector<HelpPrompt> getHelpPrompts() override;
    HelpStyle               getHelpStyle()   override;

private:
    // ── Internal helpers ──────────────────────────────────────────────────

    // Populate mEntries from the visible system list.
    void populate();

    // Navigate to index, launching a cross-fade animation when animate=true.
    void navigateTo(int newIndex, bool animate);

    // Rebuild the logo component for the entry at mCursor.
    void rebuildLogo();

    // Rebuild info-panel text components for mCursor.
    void rebuildInfoPanel();

    // Low-level render helpers.
    void renderBackground(const Transform4x4f& trans, unsigned char alpha);
    void renderSeparator  (const Transform4x4f& trans, unsigned char alpha);
    void renderArtPanel   (const Transform4x4f& trans, unsigned char alpha);
    void renderInfoPanel  (const Transform4x4f& trans, unsigned char alpha);
    void renderNavDots    (const Transform4x4f& trans, unsigned char alpha);

    // ── Per-system entry ─────────────────────────────────────────────────

    struct DashboardEntry
    {
        SystemData*                    system     = nullptr;
        std::shared_ptr<ImageComponent> logo;               // may be nullptr
        std::shared_ptr<TextComponent>  logoText;           // fallback text logo
        std::vector<GuiComponent*>      backgroundExtras;   // theme-defined extras
    };

    std::vector<DashboardEntry> mEntries;
    int mCursor;       // index into mEntries

    // ── Transition state ─────────────────────────────────────────────────

    // mFade runs from 0 (fully visible) → 1 (fully faded) during the
    // out-half of a cross-fade, then 1 → 0 for the in-half.
    float mFade;
    bool  mShowing;

    // ── Art panel components ─────────────────────────────────────────────

    // Logo for the current system (rebuilt on every navigate).
    std::shared_ptr<GuiComponent> mCurrentLogo;

    // ── Info panel components ─────────────────────────────────────────────

    TextComponent mSystemNameText;   // large — system full name
    TextComponent mGameCountText;    // medium — "N games"
    TextComponent mPlatformText;     // small  — platform / tag line

    // ── HUD bar ──────────────────────────────────────────────────────────

    std::unique_ptr<HudOverlay> mHudOverlay;

    // ── Layout constants (computed once in ctor) ─────────────────────────

    float mScreenW;
    float mScreenH;
    float mArtPanelW;    // 55 % of screen width
    float mInfoPanelX;   // mArtPanelW
    float mInfoPanelW;   // 45 % of screen width
    float mHudH;         // 7 % of screen height
    float mContentH;     // screen height minus HUD bar
};

#endif // CLON3D_VIEWS_DASHBOARD_VIEW_H
