#pragma once
#ifndef CLON3D_VIEWS_DASHBOARD_WIDGET_H
#define CLON3D_VIEWS_DASHBOARD_WIDGET_H

// Clon3D Cyberdeck OS — Dashboard Widget Base
//
// DashboardWidget is the abstract base class for all dashboard panels and HUD
// elements in the Clon3D dashboard system view.  Widgets are placed in named
// screen regions (anchors) and receive system-change notifications so they can
// update their content.
//
// Future widgets (battery, CPU load, Wi-Fi, temperature, storage, …) should
// inherit from this class.

#include "GuiComponent.h"
#include <string>

class SystemData;

class DashboardWidget : public GuiComponent
{
public:
    // Screen region where this widget is attached.
    enum class Anchor
    {
        HUD_LEFT,
        HUD_CENTER,
        HUD_RIGHT,
        PANEL_TOP_LEFT,
        PANEL_TOP_RIGHT,
        PANEL_BOTTOM_LEFT,
        PANEL_BOTTOM_RIGHT,
        OVERLAY_CENTER
    };

    explicit DashboardWidget(Window* window);
    virtual ~DashboardWidget() = default;

    // Called whenever the currently selected system changes.
    // Subclasses should override to refresh displayed content.
    virtual void onSystemChanged(SystemData* system) {}

    // Unique string identifier (e.g. "hud_clock", "hud_system_count").
    virtual const std::string& getWidgetId() const = 0;

    Anchor getAnchor() const     { return mAnchor; }
    void   setAnchor(Anchor a)   { mAnchor = a; }

protected:
    Anchor mAnchor;
};

#endif // CLON3D_VIEWS_DASHBOARD_WIDGET_H
