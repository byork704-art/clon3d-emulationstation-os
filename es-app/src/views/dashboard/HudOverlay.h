#pragma once
#ifndef CLON3D_VIEWS_DASHBOARD_HUD_OVERLAY_H
#define CLON3D_VIEWS_DASHBOARD_HUD_OVERLAY_H

// Clon3D Cyberdeck OS — HUD Overlay Widget
//
// HudOverlay is a translucent status bar rendered at the bottom of the
// DashboardView.  It holds a row of status widgets (clock, system counter,
// and placeholders for future hardware-monitor widgets).
//
// Visual design: dark semi-transparent panel, thin cyan accent line on top,
// white/grey text for status items.

#include "views/dashboard/DashboardWidget.h"
#include "components/TextComponent.h"

class HudOverlay : public DashboardWidget
{
public:
    explicit HudOverlay(Window* window);

    void onSystemChanged(SystemData* system) override;

    void update(int deltaTime) override;
    void render(const Transform4x4f& parentTrans) override;

    const std::string& getWidgetId() const override;

private:
    void updateClock();

    // Static id so ViewController can look this up by name if needed.
    static const std::string sWidgetId;

    // System counter text  (e.g. "3 / 12")
    TextComponent mSystemCountText;

    // Live clock text  (e.g. "14:37")
    TextComponent mClockText;

    // Hint label  (e.g. "A  SELECT   ←→  BROWSE")
    TextComponent mHintText;

    // How many ms have elapsed since the last clock refresh.
    int mClockTimer;

    // Total number of visible systems — refreshed in onSystemChanged.
    int mSystemIndex;
    int mSystemTotal;
};

#endif // CLON3D_VIEWS_DASHBOARD_HUD_OVERLAY_H
