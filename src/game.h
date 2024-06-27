#pragma once

#include "game_lib.h"
#include "input.h"
#include "render_interface.h"

struct GameState
{
    bool initialized = false;
    IVec2 playerPos;
};


static GameState* gameState;


extern "C"
{
    EXPORT_FN void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn);
}