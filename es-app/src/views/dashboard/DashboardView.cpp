// Clon3D Cyberdeck OS — Dashboard System Selection View
//
// See DashboardView.h for full layout description.

#include "views/dashboard/DashboardView.h"

#include "views/dashboard/HudOverlay.h"
#include "views/ViewController.h"
#include "animations/LambdaAnimation.h"
#include "renderers/Renderer.h"
#include "resources/Font.h"
#include "Log.h"
#include "Scripting.h"
#include "Settings.h"
#include "SystemData.h"
#include "ThemeData.h"
#include "Window.h"

#include <algorithm>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

DashboardView::DashboardView(Window* window)
    : GuiComponent(window)
    , mCursor(0)
    , mFade(0.0f)
    , mShowing(false)
    , mSystemNameText(window)
    , mGameCountText(window)
    , mPlatformText(window)
{
    mScreenW = (float)Renderer::getScreenWidth();
    mScreenH = (float)Renderer::getScreenHeight();

    // Layout fractions.
    mArtPanelW  = mScreenW * 0.55f;
    mInfoPanelX = mArtPanelW;
    mInfoPanelW = mScreenW * 0.45f;
    mHudH       = mScreenH * 0.07f;
    mContentH   = mScreenH - mHudH;

    setSize(mScreenW, mScreenH);

    // ── Info panel text layout ────────────────────────────────────────────

    const float infoPad   = mInfoPanelW * 0.06f;
    const float infoTextW = mInfoPanelW - infoPad * 2.0f;

    // System full name — large, white.
    mSystemNameText.setFont(Font::get(FONT_SIZE_LARGE, FONT_PATH_REGULAR));
    mSystemNameText.setColor(0xFFFFFFFF);
    mSystemNameText.setSize(infoTextW, 0.0f);
    mSystemNameText.setPosition(mInfoPanelX + infoPad, mContentH * 0.20f);
    mSystemNameText.setHorizontalAlignment(ALIGN_LEFT);

    // Game count — medium, tinted cyan.
    mGameCountText.setFont(Font::get(FONT_SIZE_MEDIUM, FONT_PATH_REGULAR));
    mGameCountText.setColor(0x00D4FFFF);
    mGameCountText.setSize(infoTextW, 0.0f);
    mGameCountText.setPosition(mInfoPanelX + infoPad, mContentH * 0.42f);
    mGameCountText.setHorizontalAlignment(ALIGN_LEFT);

    // Platform hint — small, grey.
    mPlatformText.setFont(Font::get(FONT_SIZE_SMALL, FONT_PATH_LIGHT));
    mPlatformText.setColor(0x7799AAFF);
    mPlatformText.setSize(infoTextW, 0.0f);
    mPlatformText.setPosition(mInfoPanelX + infoPad, mContentH * 0.52f);
    mPlatformText.setHorizontalAlignment(ALIGN_LEFT);

    // ── HUD overlay ───────────────────────────────────────────────────────

    mHudOverlay = std::unique_ptr<HudOverlay>(new HudOverlay(window));
    mHudOverlay->setPosition(0.0f, mScreenH - mHudH);

    // ── Populate entries ─────────────────────────────────────────────────

    populate();
}

// ─────────────────────────────────────────────────────────────────────────────

DashboardView::~DashboardView()
{
    for (auto& e : mEntries)
        for (auto* extra : e.backgroundExtras)
            delete extra;
}

// ─────────────────────────────────────────────────────────────────────────────
// Populate
// ─────────────────────────────────────────────────────────────────────────────

void DashboardView::populate()
{
    // Clean up old entries first.
    for (auto& e : mEntries)
        for (auto* extra : e.backgroundExtras)
            delete extra;
    mEntries.clear();

    for (auto* sys : SystemData::sSystemVector)
    {
        if (!sys->isVisible())
            continue;

        DashboardEntry e;
        e.system = sys;

        const std::shared_ptr<ThemeData>& theme = sys->getTheme();

        // Build logo image from theme "system/logo" element.
        const ThemeData::ThemeElement* logoElem =
            theme->getElement("system", "logo", "image");

        if (logoElem)
        {
            std::string path = logoElem->get<std::string>("path");
            std::string defPath = logoElem->has("default")
                ? logoElem->get<std::string>("default") : "";
            bool hasPath = (!path.empty() &&
                ResourceManager::getInstance()->fileExists(path));
            bool hasDef  = (!defPath.empty() &&
                ResourceManager::getInstance()->fileExists(defPath));

            if (hasPath || hasDef)
            {
                ImageComponent* img = new ImageComponent(mWindow, false, false);
                // Max size fits within the art panel with comfortable padding.
                img->setMaxSize(mArtPanelW * 0.80f, mContentH * 0.55f);
                img->applyTheme(theme, "system", "logo",
                    ThemeFlags::PATH | ThemeFlags::COLOR);
                img->setOrigin(0.5f, 0.5f);
                e.logo = std::shared_ptr<ImageComponent>(img);
            }
        }

        // Fallback: text logo using system full name.
        if (!e.logo)
        {
            TextComponent* txt = new TextComponent(
                mWindow,
                sys->getFullName(),
                Font::get(FONT_SIZE_LARGE, FONT_PATH_LIGHT),
                0xFFFFFFFF,
                ALIGN_CENTER);
            txt->setSize(mArtPanelW * 0.80f, 0.0f);
            txt->setOrigin(0.5f, 0.5f);
            e.logoText = std::shared_ptr<TextComponent>(txt);
        }

        // Background extras from theme.
        e.backgroundExtras =
            ThemeData::makeExtras(theme, "system", mWindow);
        std::stable_sort(e.backgroundExtras.begin(),
            e.backgroundExtras.end(),
            [](GuiComponent* a, GuiComponent* b) {
                return b->getZIndex() > a->getZIndex();
            });

        mEntries.push_back(std::move(e));
    }

    if (!mEntries.empty())
    {
        if (mCursor >= (int)mEntries.size())
            mCursor = (int)mEntries.size() - 1;
        rebuildInfoPanel();
        mHudOverlay->onSystemChanged(mEntries[mCursor].system);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Navigation
// ─────────────────────────────────────────────────────────────────────────────

void DashboardView::goToSystem(SystemData* system, bool animate)
{
    for (int i = 0; i < (int)mEntries.size(); ++i)
    {
        if (mEntries[i].system == system)
        {
            navigateTo(i, animate);
            return;
        }
    }
}

SystemData* DashboardView::getSelected() const
{
    if (mEntries.empty())
        return nullptr;
    return mEntries[mCursor].system;
}

void DashboardView::navigateTo(int newIndex, bool animate)
{
    if (newIndex == mCursor && !animate)
    {
        rebuildInfoPanel();
        return;
    }

    if (!animate)
    {
        mCursor = newIndex;
        mFade = 0.0f;
        cancelAnimation(0);
        rebuildInfoPanel();
        if (!mEntries.empty())
            mHudOverlay->onSystemChanged(mEntries[mCursor].system);
        return;
    }

    // Cross-fade: animate mFade 0→1, swap cursor, animate 1→0.
    const int FADE_MS = 120;

    cancelAnimation(0);
    int targetCursor = newIndex;

    auto fadeOut = [this](float t) {
        mFade = t;
    };

    setAnimation(
        new LambdaAnimation(fadeOut, FADE_MS),
        0,
        [this, targetCursor, FADE_MS]() {
            mCursor = targetCursor;
            rebuildInfoPanel();
            if (!mEntries.empty())
                mHudOverlay->onSystemChanged(mEntries[mCursor].system);

            auto fadeIn = [this](float t) {
                mFade = 1.0f - t;
            };
            setAnimation(new LambdaAnimation(fadeIn, FADE_MS), 0, nullptr);
        });
}

// ─────────────────────────────────────────────────────────────────────────────
// Rebuild helpers
// ─────────────────────────────────────────────────────────────────────────────

void DashboardView::rebuildInfoPanel()
{
    if (mEntries.empty())
        return;

    SystemData* sys = mEntries[mCursor].system;

    mSystemNameText.setText(sys->getFullName());

    unsigned int count = sys->getDisplayedGameCount();
    std::string countStr = std::to_string(count)
        + (count == 1 ? " game" : " games");
    mGameCountText.setText(countStr);

    // Platform tag line: use theme folder name as a tidy platform identifier.
    mPlatformText.setText(sys->getName());
}

// ─────────────────────────────────────────────────────────────────────────────
// Input
// ─────────────────────────────────────────────────────────────────────────────

bool DashboardView::input(InputConfig* config, Input input)
{
    if (mEntries.empty())
        return false;

    if (input.value != 0)
    {
        if (config->isMappedLike("left", input))
        {
            int next = (mCursor - 1 + (int)mEntries.size()) % (int)mEntries.size();
            navigateTo(next, true);
            Scripting::fireEvent("system-select",
                mEntries[mCursor].system->getName(), "dashboard-nav");
            return true;
        }
        if (config->isMappedLike("right", input))
        {
            int next = (mCursor + 1) % (int)mEntries.size();
            navigateTo(next, true);
            Scripting::fireEvent("system-select",
                mEntries[mCursor].system->getName(), "dashboard-nav");
            return true;
        }
        if (config->isMappedTo("a", input))
        {
            ViewController::get()->goToGameList(getSelected());
            return true;
        }
        if (config->isMappedTo("x", input))
        {
            SystemData* rnd = SystemData::getRandomSystem();
            if (rnd)
                goToSystem(rnd, true);
            return true;
        }
        if (!Settings::getInstance()->getBool("DisableKidStartMenu")
            && config->isMappedTo("select", input)
            && Settings::getInstance()->getBool("ScreenSaverControls"))
        {
            mWindow->startScreenSaver();
            mWindow->renderScreenSaver();
            return true;
        }
    }
    else
    {
        // Button released.
        if (config->isMappedLike("left", input) ||
            config->isMappedLike("right", input))
        {
            Scripting::fireEvent("system-select",
                mEntries[mCursor].system->getName(), "input");
        }
    }

    return GuiComponent::input(config, input);
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

void DashboardView::update(int deltaTime)
{
    mHudOverlay->update(deltaTime);
    GuiComponent::update(deltaTime); // handles animations
}

// ─────────────────────────────────────────────────────────────────────────────
// Show / Hide
// ─────────────────────────────────────────────────────────────────────────────

void DashboardView::onShow()
{
    mShowing = true;
    GuiComponent::onShow();
}

void DashboardView::onHide()
{
    mShowing = false;
    GuiComponent::onHide();
}

// ─────────────────────────────────────────────────────────────────────────────
// Help
// ─────────────────────────────────────────────────────────────────────────────

std::vector<HelpPrompt> DashboardView::getHelpPrompts()
{
    std::vector<HelpPrompt> prompts;
    prompts.push_back(HelpPrompt("left/right", "browse"));
    prompts.push_back(HelpPrompt("a",          "select"));
    prompts.push_back(HelpPrompt("x",          "random"));
    return prompts;
}

HelpStyle DashboardView::getHelpStyle()
{
    if (!mEntries.empty())
    {
        HelpStyle style;
        style.applyTheme(mEntries[mCursor].system->getTheme(), "system");
        return style;
    }
    return GuiComponent::getHelpStyle();
}

// ─────────────────────────────────────────────────────────────────────────────
// Render
// ─────────────────────────────────────────────────────────────────────────────

void DashboardView::render(const Transform4x4f& parentTrans)
{
    if (mEntries.empty())
        return;

    Transform4x4f trans = parentTrans * getTransform();

    // Content alpha is driven by mFade (0 = fully visible, 1 = fully faded).
    unsigned char alpha = (unsigned char)(255.0f * (1.0f - mFade));

    renderBackground(trans, alpha);
    renderSeparator  (trans, alpha);
    renderArtPanel   (trans, alpha);
    renderInfoPanel  (trans, alpha);
    renderNavDots    (trans, alpha);

    // HUD bar is always fully opaque (independent of transition fade).
    mHudOverlay->render(trans);
}

// ─────────────────────────────────────────────────────────────────────────────
// Render helpers
// ─────────────────────────────────────────────────────────────────────────────

void DashboardView::renderBackground(const Transform4x4f& trans,
                                     unsigned char alpha)
{
    Renderer::setMatrix(trans);

    // Deep dark-blue/near-black base.
    unsigned int base = (0x05080F00) | alpha;
    Renderer::drawRect(0.0f, 0.0f, mScreenW, mScreenH, base, base);

    // Background extras from the current system's theme.
    const auto& extras = mEntries[mCursor].backgroundExtras;
    for (auto* extra : extras)
    {
        extra->setOpacity(alpha);
        extra->render(trans);
    }

    // Subtle gradient overlay so artwork always reads well.
    unsigned int gradTop = (0x00000000) | (unsigned char)(alpha * 0.0f);
    unsigned int gradBot = (0x00000000) | (unsigned char)(alpha * 0.55f);
    Renderer::drawRect(0.0f, 0.0f, mScreenW, mContentH,
        gradTop, gradBot, false); // vertical gradient
}

void DashboardView::renderSeparator(const Transform4x4f& trans,
                                    unsigned char alpha)
{
    Renderer::setMatrix(trans);

    // Vertical cyan accent line between art and info panels.
    unsigned int lineColor = (0x00D4FF00) | (unsigned char)(alpha * 0.60f);
    Renderer::drawRect(mArtPanelW - 1.0f, 0.0f, 2.0f, mContentH,
        lineColor, lineColor);
}

void DashboardView::renderArtPanel(const Transform4x4f& trans,
                                   unsigned char alpha)
{
    const DashboardEntry& entry = mEntries[mCursor];

    // Centre of the art panel (vertically centred in the content area,
    // with a slight upward bias for visual weight).
    const float cx = mArtPanelW * 0.50f;
    const float cy = mContentH  * 0.45f;

    if (entry.logo)
    {
        entry.logo->setOpacity(alpha);
        // Position around the centre of the art panel.
        Vector2f halfSz = entry.logo->getSize() * 0.5f;
        entry.logo->setPosition(cx - halfSz.x(), cy - halfSz.y());
        entry.logo->render(trans);
    }
    else if (entry.logoText)
    {
        entry.logoText->setOpacity(alpha);
        Vector2f sz = entry.logoText->getSize();
        entry.logoText->setPosition(cx - sz.x() * 0.5f, cy - sz.y() * 0.5f);
        entry.logoText->render(trans);
    }
}

void DashboardView::renderInfoPanel(const Transform4x4f& trans,
                                    unsigned char alpha)
{
    Renderer::setMatrix(trans);

    // Semi-transparent panel background.
    unsigned int panelBg = (unsigned char)(alpha * 0.45f);
    Renderer::drawRect(mInfoPanelX, 0.0f, mInfoPanelW, mContentH,
        panelBg, panelBg);

    mSystemNameText.setOpacity(alpha);
    mGameCountText .setOpacity(alpha);
    mPlatformText  .setOpacity(alpha);

    mSystemNameText.render(trans);
    mGameCountText .render(trans);
    mPlatformText  .render(trans);
}

void DashboardView::renderNavDots(const Transform4x4f& trans,
                                  unsigned char alpha)
{
    if (mEntries.size() <= 1)
        return;

    Renderer::setMatrix(trans);

    // Clamp displayed dots to avoid cluttering very large system lists.
    const int maxDots   = 12;
    int       totalSys  = (int)mEntries.size();
    int       dotCount  = Math::min(totalSys, maxDots);

    const float dotR    = 4.0f;
    const float dotGap  = 12.0f;
    const float row     = mContentH - 20.0f;
    const float totalW  = dotCount * (dotR * 2.0f) + (dotCount - 1) * dotGap;
    const float startX  = (mArtPanelW - totalW) * 0.5f;

    // When mCursor is in the overflow region, keep the indicator centred on
    // the proportional position rather than showing exact dot.
    int activeDot = (dotCount < totalSys)
        ? (int)((float)mCursor / (float)(totalSys - 1) * (dotCount - 1) + 0.5f)
        : mCursor;

    for (int i = 0; i < dotCount; ++i)
    {
        float x = startX + i * (dotR * 2.0f + dotGap);
        bool  active = (i == activeDot);

        unsigned int col = active
            ? ((0x00D4FF00) | alpha)           // active: cyan
            : ((0x44556600) | (unsigned char)(alpha / 2)); // inactive: dim

        Renderer::drawRect(x, row - dotR, dotR * 2.0f, dotR * 2.0f, col, col);
    }
}
