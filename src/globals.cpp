#include "globals.hpp"

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::globals {

entt::registry registry;

float TOTAL_TIME = 0.0;
float FRAME_DT = 0.0;
bool WINDOW_SHOULD_CLOSE = false;
GameState GAME_STATE = GameState::PLAY;
RenderOptions RENDER_OPTIONS;

void update() {
    FRAME_DT = GetFrameTime();
    TOTAL_TIME += FRAME_DT;

    bool is_alt_f4_pressed = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    WINDOW_SHOULD_CLOSE = (WindowShouldClose() || is_alt_f4_pressed);

    if (IsKeyPressed(KEY_F1)) {
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
