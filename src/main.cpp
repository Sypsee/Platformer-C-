#include "game_lib.h"
#include "input.h"
#include "platform.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"

// Windows platform

#ifdef _WIN32
#include "win32_platform.cpp"
#endif // End of windows platform

#include "gl_renderer.cpp"

int main()
{
    BumpAllocator transientStorage = make_bump_alloctor(MB(50));
    platform_create_window(800, 600, "Platformer C");
    input.screenSizeX = 800;
    input.screenSizeY = 600;
    
    gl_init(&transientStorage);

    while (running)
    {
        // Update
        platform_update_window();
        gl_render();
    }

    return 0;
}