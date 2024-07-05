#include "globals.hpp"

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::globals {

entt::registry registry;

float FRAME_DT = 0.0;
bool WINDOW_SHOULD_CLOSE = false;
GameState GAME_STATE = GameState::PLAY;

void update() {
    // FRAME_DT
    FRAME_DT = GetFrameTime();

    // WINDOW_SHOULD_CLOSE
    bool is_alt_f4_pressed = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    WINDOW_SHOULD_CLOSE = (WindowShouldClose() || is_alt_f4_pressed);

    // MODE
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (GAME_STATE == GameState::PLAY) {
            GAME_STATE = GameState::EDITOR;
            EnableCursor();
        } else if (GAME_STATE == GameState::EDITOR) {
            GAME_STATE = GameState::PLAY;
            DisableCursor();
        }
    }
}

}  // namespace soft_tissues::globals
