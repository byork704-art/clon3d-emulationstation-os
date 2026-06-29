# Clon3D Cyberdeck OS — Rendering Architecture

> **Status:** Phase 1 — Dashboard System-Select View  
> **Branch:** `copilot/copilotimport-retropie-emulationstation-source`

---

## Overview

Clon3D Cyberdeck OS is built on top of the RetroPie EmulationStation engine.
Rather than replacing the engine, Phase 1 introduces a **new rendering path**
that co-exists alongside the original code.  The dashboard mode is activated
with a single settings flag and leaves the legacy carousel path completely
untouched.

---

## Setting

```xml
<!-- ~/.emulationstation/es_settings.xml -->
<bool name="DashboardMode" value="true" />
```

When `DashboardMode` is `false` (the default), the engine behaves identically
to upstream RetroPie EmulationStation — the horizontal carousel `SystemView`
is rendered as normal.

When `DashboardMode` is `true`, `ViewController` instantiates a
`DashboardView` in place of `SystemView` and routes all system-select
rendering through it.

---

## Class Hierarchy

```
GuiComponent  (es-core)
│
├── SystemView  [legacy carousel — unchanged]
│   └── IList<SystemViewData, SystemData*>
│
└── DashboardView  [Clon3D Phase 1]  ← NEW
    │
    ├── DashboardWidget (abstract)   ← NEW
    │   └── HudOverlay               ← NEW  (bottom status bar)
    │
    ├── ImageComponent  (system logo / artwork)
    └── TextComponent × 3  (name, count, platform)
```

---

## ViewController Routing

`ViewController::goToSystemView(SystemData*)` now branches on the setting:

```
DashboardMode == false  →  getSystemListView()  →  SystemView (carousel)
DashboardMode == true   →  getDashboardView()   →  DashboardView (dashboard)
```

`ViewController::render()` follows the same branch so only one system-select
view is drawn per frame.  Game-list views are unchanged in both modes.

Both `mSystemListView` and `mDashboardView` live inside `ViewController` as
`std::shared_ptr` members.  Only the active one is ever rendered; the inactive
one is created lazily on first use.

---

## DashboardView Layout

```
┌──────────────────────────────────────────────────────────────┐ ← screen top
│  Background  (full screen)                                   │
│  Theme background-extras drawn here (z-sorted)               │
│  Dark gradient overlay applied on top                        │
│                                                              │
│  ┌─────────────────────────┐  │  ┌─────────────────────────┐ │
│  │  Art Panel  (55 %)      │  │  │  Info Panel  (45 %)     │ │
│  │                         │  │  │                         │ │
│  │   System logo / artwork │  │  │  System full name       │ │
│  │   (centred, max-fit)    │  │  │  (FONT_SIZE_LARGE)      │ │
│  │                         │  │  │                         │ │
│  │                         │  │  │  N games                │ │
│  │                         │  │  │  (FONT_SIZE_MEDIUM,     │ │
│  │                         │  │  │   cyan)                 │ │
│  │                         │  │  │                         │ │
│  │                         │  │  │  system-name identifier │ │
│  │                         │  │  │  (FONT_SIZE_SMALL, grey)│ │
│  │                         │  │  │                         │ │
│  │  ● ● ◉ ● ●  (nav dots)  │  │  │                         │ │
│  └─────────────────────────┘  │  └─────────────────────────┘ │
│  ──────────────────────────────────────────────────────────  │
│  HUD bar  (7 % height)  — system N/M  |  hints  |  clock    │
└──────────────────────────────────────────────────────────────┘ ← screen bottom
```

### Colour Palette (defaults)

| Element                 | RGBA hex     | Notes                          |
|-------------------------|--------------|--------------------------------|
| Background base         | `0x05080FFF` | Near-black blue-tinted         |
| Gradient overlay        | `0x00000000→0x0000008C` | Top transparent → 55 % black |
| Separator line          | `0x00D4FF99` | Cyan, 60 % opacity             |
| System name             | `0xFFFFFFFF` | White                          |
| Game count              | `0x00D4FFFF` | Cyan                           |
| Platform identifier     | `0x7799AAFF` | Muted blue-grey                |
| HUD bar background      | `0x05080FD0` | Dark, 82 % opacity             |
| HUD accent line         | `0x00D4FF80` | Cyan, 50 % opacity             |
| HUD system-count text   | `0xAADDFFFF` | Light blue                     |
| HUD clock text          | `0xEEEEEEFF` | Near-white                     |
| HUD hint text           | `0x667788FF` | Muted                          |
| Nav dot (active)        | `0x00D4FFAA` | Cyan                           |
| Nav dot (inactive)      | `0x44556666` | Dim grey                       |

---

## Transition System

System navigation uses a **cross-fade** driven by the existing `LambdaAnimation`
infrastructure:

1. `navigateTo(index, animate=true)` starts an animation on slot 0.
2. `mFade` runs **0 → 1** over 120 ms (content fades out).
3. At the midpoint callback, `mCursor` is updated and components rebuilt.
4. A second animation runs **1 → 0** over 120 ms (new content fades in).

All content (logo, info text, background extras) has its opacity multiplied
by `(1 - mFade)` so the transition is uniform.  The HUD bar remains at full
opacity throughout.

---

## Widget System

`DashboardWidget` is an abstract `GuiComponent` subclass with two additions:

- **`Anchor`** enum — describes where on screen the widget lives
  (`HUD_LEFT`, `HUD_CENTER`, `HUD_RIGHT`, `PANEL_*`, `OVERLAY_CENTER`).
- **`onSystemChanged(SystemData*)`** virtual — called when the cursor moves
  so the widget can refresh its content.

### Implemented Widgets (Phase 1)

| Class        | Anchor       | Content                               |
|--------------|--------------|---------------------------------------|
| `HudOverlay` | `HUD_CENTER` | System N/M counter, clock, hint text  |

### Planned Widgets (Future Phases)

- `HudBattery` — battery level + charging indicator
- `HudCpuTemp` — CPU temperature (Pi thermal zone)
- `HudWifi` — Wi-Fi signal strength
- `HudStorage` — storage usage bar
- `InfoLastPlayed` — last-played game thumbnail + title
- `InfoRecentlyAdded` — recently-added games strip
- `InfoPlayStats` — total play time, session count

---

## Theme Compatibility

DashboardView reads the same theme elements as the original SystemView:

| Theme element               | Used for                     |
|-----------------------------|------------------------------|
| `system / logo` (image)     | System artwork in art panel  |
| `system / *` (extras)       | Background extras layer      |
| `system` (HelpStyle)        | Help prompt colours          |

No new theme XML tags are introduced in Phase 1.  All new visual chrome
(colours, layout, HUD) is hard-coded in C++ and will be made theme-overridable
in a future phase via new `<dashboard>` XML tags (see `THEMES.md` extension
when that work lands).

---

## Building

Dashboard mode is compiled in by default — no build flags required.  To
disable the feature entirely, the `DashboardMode` setting defaults to `false`
so existing deployments are unaffected.

```bash
# Standard Raspberry Pi build (dashboard code compiled, mode off by default)
mkdir build && cd build
cmake .. -DUSE_MESA_GLES=On -DRPI=On
make -j$(nproc)
```

---

## Files Added / Changed in Phase 1

### New

| File                                               | Purpose                            |
|----------------------------------------------------|------------------------------------|
| `es-app/src/views/dashboard/DashboardWidget.h/.cpp` | Abstract widget base class        |
| `es-app/src/views/dashboard/HudOverlay.h/.cpp`     | Bottom HUD status bar              |
| `es-app/src/views/dashboard/DashboardView.h/.cpp`  | Fullscreen dashboard system view   |
| `docs/RENDERING_ARCHITECTURE.md`                   | This document                      |

### Modified

| File                                    | Change                                         |
|-----------------------------------------|------------------------------------------------|
| `es-core/src/Settings.cpp`              | Add `DashboardMode` default (`false`)          |
| `es-app/src/views/ViewController.h`     | Add `DashboardView` forward-decl + member      |
| `es-app/src/views/ViewController.cpp`   | Route system-select + render to `DashboardView`|
| `es-app/CMakeLists.txt`                 | Register new source files                      |
