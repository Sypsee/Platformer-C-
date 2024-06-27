#include "game_lib.h"
#include "input.h"
#include "game.h"
#include "platform.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"

// Windows platform

#ifdef _WIN32
#include "win32_platform.cpp"
#endif // End of windows platform

#include "gl_renderer.cpp"


// ----------------------- GAME DLL STUFF -----------------------
// This is the function pointer to update_game in game.cpp
typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;

void reload_game_dll(BumpAllocator* transientStorage);




int main()
{
    BumpAllocator transientStorage = make_bump_alloctor(MB(50));
    BumpAllocator persistantStorage = make_bump_alloctor(MB(50));
    
    input = (Input*)bump_alloc(&persistantStorage, sizeof(Input));
    renderData = (RenderData*)bump_alloc(&persistantStorage, sizeof(RenderData));
    gameState = (GameState*)bump_alloc(&persistantStorage, sizeof(GameState));

    if (!input)
    {
        GM_ERROR("Failed to allocate Input");
        return -1;
    }

    if (!renderData)
    {
        GM_ERROR("Failed to allocate Render Data");
        return -1;
    }

    if (!gameState)
    {
        GM_ERROR("Failed to allocate Game State");
        return -1;
    }

    platform_create_window(1000, 600, "Platformer C");
    input->screenSizeX = 1000;
    input->screenSizeY = 600;
    
    gl_init(&transientStorage);

    while (running)
    {
        // Update

        reload_game_dll(&transientStorage);

        platform_update_window();
        update_game(gameState, renderData, input);
        gl_render();

        platform_swap_buffers();

        transientStorage.used = 0;
    }

    return 0;
}

void update_game(GameState* gameStateIn, RenderData* renderDataIn, Input* inputIn)
{
    update_game_ptr(gameStateIn, renderDataIn, inputIn);
}

void reload_game_dll(BumpAllocator* transientStorage)
{
    static void* gameDLL;
    static long long lastEditTimestampGameDLL;

    long long currentTimestampGameDLL = get_timestamp("game.dll");
    if (currentTimestampGameDLL > lastEditTimestampGameDLL)
    {
        if (gameDLL)
        {
            bool freeResult = platform_free_dynamic_library(gameDLL);
            GM_ASSERT(freeResult, "Failed to free game.dll!");
            gameDLL = nullptr;
            GM_TRACE("Freed game.dll");
        }

        while (!copy_file("game.dll", "game_load.dll", transientStorage))
        {
            Sleep(10);
        }
        GM_TRACE("Copied game.dll to game_load.dll successfully!");

        gameDLL = platform_load_dynamic_library("game_load.dll");
        GM_ASSERT(gameDLL, "Could not laod game.dll!");

        update_game_ptr = (update_game_type*)platform_load_dynamic_function(gameDLL, "update_game");
        GM_ASSERT(update_game_ptr, "Failed to load update_game function!");

        lastEditTimestampGameDLL = currentTimestampGameDLL;
    }
}