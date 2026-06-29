// Clon3D Cyberdeck OS — HUD Overlay Widget

#include "views/dashboard/HudOverlay.h"

#include "renderers/Renderer.h"
#include "resources/Font.h"
#include "SystemData.h"

#include <ctime>
#include <sstream>
#include <iomanip>

// ─────────────────────────────────────────────────────────────────────────────

const std::string HudOverlay::sWidgetId = "hud_overlay";

// ─────────────────────────────────────────────────────────────────────────────

HudOverlay::HudOverlay(Window* window)
    : DashboardWidget(window)
    , mSystemCountText(window)
    , mClockText(window)
    , mHintText(window)
    , mClockTimer(0)
    , mSystemIndex(0)
    , mSystemTotal(0)
{
    setAnchor(Anchor::HUD_CENTER);

    const float sw = (float)Renderer::getScreenWidth();
    const float sh = (float)Renderer::getScreenHeight();

    // HUD bar occupies the bottom 7 % of the screen.
    const float hudH = sh * 0.07f;
    setSize(sw, hudH);

    auto fontSmall = Font::get(FONT_SIZE_SMALL, FONT_PATH_REGULAR);
    auto fontMini  = Font::get(FONT_SIZE_MINI,  FONT_PATH_REGULAR);

    // --- System counter (left side) ---
    mSystemCountText.setFont(fontSmall);
    mSystemCountText.setColor(0xAADDFFFF);
    mSystemCountText.setSize(sw * 0.20f, hudH);
    mSystemCountText.setPosition(sw * 0.02f, 0.0f);
    mSystemCountText.setHorizontalAlignment(ALIGN_LEFT);
    mSystemCountText.setVerticalAlignment(ALIGN_CENTER);

    // --- Clock (right side) ---
    mClockText.setFont(fontSmall);
    mClockText.setColor(0xEEEEEEFF);
    mClockText.setSize(sw * 0.15f, hudH);
    mClockText.setPosition(sw * 0.83f, 0.0f);
    mClockText.setHorizontalAlignment(ALIGN_RIGHT);
    mClockText.setVerticalAlignment(ALIGN_CENTER);

    // --- Hint bar (centre) ---
    mHintText.setFont(fontMini);
    mHintText.setColor(0x667788FF);
    mHintText.setSize(sw * 0.40f, hudH);
    mHintText.setPosition(sw * 0.30f, 0.0f);
    mHintText.setHorizontalAlignment(ALIGN_CENTER);
    mHintText.setVerticalAlignment(ALIGN_CENTER);
    mHintText.setText("A  SELECT     \u2190 \u2192  BROWSE     START  MENU");

    updateClock();
}

// ─────────────────────────────────────────────────────────────────────────────

const std::string& HudOverlay::getWidgetId() const
{
    return sWidgetId;
}

// ─────────────────────────────────────────────────────────────────────────────

void HudOverlay::onSystemChanged(SystemData* system)
{
    if (!system)
        return;

    // Count visible systems.
    mSystemTotal = 0;
    mSystemIndex = 0;
    for (auto* s : SystemData::sSystemVector)
    {
        if (s->isVisible())
        {
            if (s == system)
                mSystemIndex = mSystemTotal + 1;
            ++mSystemTotal;
        }
    }

    std::string label = std::to_string(mSystemIndex) + " / " + std::to_string(mSystemTotal);
    mSystemCountText.setText(label);
}

// ─────────────────────────────────────────────────────────────────────────────

void HudOverlay::update(int deltaTime)
{
    mClockTimer += deltaTime;
    if (mClockTimer >= 10000) // refresh every 10 s
    {
        mClockTimer = 0;
        updateClock();
    }
    GuiComponent::update(deltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────

void HudOverlay::updateClock()
{
    std::time_t t = std::time(nullptr);
    std::tm*    tm = std::localtime(&t);
    if (!tm)
        return;

    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M", tm);
    mClockText.setText(buf);
}

// ─────────────────────────────────────────────────────────────────────────────

void HudOverlay::render(const Transform4x4f& parentTrans)
{
    Transform4x4f trans = parentTrans * getTransform();
    Renderer::setMatrix(trans);

    const float w = getSize().x();
    const float h = getSize().y();

    // Semi-transparent dark background.
    Renderer::drawRect(0.0f, 0.0f, w, h, 0x05080FD0, 0x05080FD0);

    // Thin cyan accent line along the top.
    Renderer::drawRect(0.0f, 0.0f, w, 1.5f, 0x00D4FF80, 0x00D4FF80);

    // Render text children in widget-local space.
    mSystemCountText.render(trans);
    mHintText.render(trans);
    mClockText.render(trans);
}
