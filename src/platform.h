#pragma once


static bool running = true;

bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_function(char* funcName);