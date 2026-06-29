// SPDX-License-Identifier: MIT
// Clon3D EmulationStation — GamelistView.h
// Game-list view for a single system.
//
// This view is shown after the user selects a system from SystemView.
// It displays a scrollable list of game entries from the gamelist.xml
// file for the selected system.
//
// Note: Full implementation is deferred to Stage 2.
// The header is provided here to establish the interface and allow
// ViewController to reference it.

#pragma once

#include "../components/IComponent.h"
#include <string>
#include <vector>

namespace clon3d {

/// Metadata for one game.
struct GameDescriptor {
    std::string path;        ///< Absolute path to the ROM file.
    std::string name;        ///< Display name.
    std::string description; ///< Short description.
    std::string imagePath;   ///< Box art / screenshot path.
    std::string developer;
    std::string publisher;
    std::string genre;
    std::string releaseDate; ///< ISO 8601 format.
    float       rating{0.0f};
    bool        favorite{false};
    bool        hidden{false};
};

/// Scrollable list of games for a single system.
class GamelistView : public IComponent {
public:
    explicit GamelistView(std::string systemId);

    // -------------------------------------------------------------------------
    // IComponent interface
    // -------------------------------------------------------------------------

    const char* typeName() const override { return "gamelistView"; }

    Rect bounds() const override { return mBounds; }
    void setBounds(const Rect& r) override { mBounds = r; }

    void update(float dt) override;
    void render(IRenderer& renderer) override;
    bool onInput(const InputEvent& event) override;

    // -------------------------------------------------------------------------
    // Data
    // -------------------------------------------------------------------------

    /// Parse a gamelist.xml file and populate the game list.
    bool loadGamelist(const std::string& gamelistPath);

    /// Return all loaded games.
    const std::vector<GameDescriptor>& games() const { return mGames; }

    /// Currently selected game, or nullptr.
    const GameDescriptor* selectedGame() const;

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------

    using GameLaunchCallback = std::function<void(const GameDescriptor&)>;
    void setOnGameLaunch(GameLaunchCallback cb) { mOnLaunch = std::move(cb); }

private:
    Rect                      mBounds{0.0f, 0.0f, 1.0f, 1.0f};
    std::string               mSystemId;
    std::vector<GameDescriptor> mGames;
    size_t                    mSelectedIndex{0};
    GameLaunchCallback        mOnLaunch;
};

} // namespace clon3d
