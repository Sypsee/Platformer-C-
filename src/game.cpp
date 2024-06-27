#include "game.h"
#include "game_lib.h"
#include "assets.h"


// ---------------------- GAME CONSTANTS ----------------------

constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;

// ---------------------- END  CONSTANTS ----------------------



EXPORT_FN void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn)
{
    if (renderData != renderDataIn)
    {
        gameState = gameStateIn;
        renderData = renderDataIn;
        input = inputIn;
    }

    if (!gameState->initialized)
    {
        renderData->gameCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
        renderData->gameCamera.pos.x = 0;
        gameState->initialized = true;
    }

    draw_sprite(SPRITE_DICE, {0.f, 0.f});
}