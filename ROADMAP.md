# Clon3D EmulationStation — Engine Roadmap

This roadmap outlines planned engine enhancements for the Clon3D fork of EmulationStation.
All items are targeted at the `clon3d-engine-import` branch and its successors.

---

## Phase 1 — Foundation (current)

- [x] Import upstream RetroPie EmulationStation source tree
- [x] Preserve upstream git history
- [x] Add Raspberry Pi build instructions (`BUILD.md`)
- [ ] Establish upstream merge workflow
- [ ] Configure submodule tracking for `external/` dependencies

---

## Phase 2 — Custom Carousel

- [ ] **Theme-controlled carousel layout** — allow themes to declare carousel item size, spacing, arc curvature, and Z-depth via `<carousel>` XML elements
- [ ] **Multi-row carousel** — optional second row for sub-category browsing
- [ ] **Animated cover art transitions** — configurable easing curves for item scroll animation
- [ ] **Video preview integration** — show animated preview in carousel background while browsing
- [ ] **Per-system carousel overrides** — themes can define different carousel behaviors per system

---

## Phase 3 — Theme-Controlled Layouts

- [ ] **Flexible layout engine** — replace hard-coded view positions with theme-defined anchors and flex containers
- [ ] **Named slots** — themes declare named slots (e.g., `header`, `footer`, `sidebar`) for components to attach to
- [ ] **Responsive scaling** — automatic UI scaling for 1080p, 4K, and non-standard resolutions
- [ ] **Dark/light mode switching** — runtime theme variant support via a single `<variant>` flag
- [ ] **Z-layer ordering** — explicit `z-index` on all theme components

---

## Phase 4 — HUD Widgets

- [ ] **Clock widget** — optional on-screen clock with configurable format and position
- [ ] **Battery indicator** — display battery level for portable Pi builds
- [ ] **Currently playing widget** — overlay showing last-launched game name and system
- [ ] **Achievement/trophy widget** — integration point for RetroAchievements HUD overlay
- [ ] **Network status indicator** — Wi-Fi/ethernet icon in status bar

---

## Phase 5 — Input & Accessibility

- [ ] **Gesture support** — swipe gestures on touchscreen-enabled Pi builds
- [ ] **Keyboard shortcut remapping** — user-editable keyboard bindings stored in `es_input.cfg`
- [ ] **Screen reader hooks** — accessibility text output for visually impaired users
- [ ] **Deadzone and sensitivity tuning** — per-controller analog stick calibration in the UI

---

## Phase 6 — Scraper & Metadata Enhancements

- [ ] **Parallel scraping** — multi-threaded image/metadata fetching
- [ ] **Local artwork fallback** — scan local `media/` folder before hitting remote APIs
- [ ] **Custom metadata fields** — user-defined tags (e.g., `<players>`, `<coop>`) surfaced in the UI
- [ ] **Collection tags** — tag-based dynamic collections (e.g., "All 4-player games")

---

## Long-term / Experimental

- [ ] **Wayland support** — replace X11/SDL display backend with a Wayland compositor target for Pi 5
- [ ] **Vulkan renderer** — optional Vulkan rendering path for Pi 4/5 with Mesa V3D
- [ ] **Plugin system** — dynamically loadable `.so` modules for adding views and scrapers without core modification
- [ ] **Lua scripting hooks** — lightweight scripting layer for theme-driven logic and animations

---

## Contributing

To propose a new feature or roadmap item, open an issue on the repository.
Engine changes go on the `clon3d-engine-import` branch (and future development branches).
Do not commit theme files to this repository; themes live in a separate repository.
