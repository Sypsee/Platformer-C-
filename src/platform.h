#pragma once


static bool running = true;

bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_function(char* funcName);
void platform_swap_buffers();
bool platform_free_dynamic_library(void* dll);
void* platform_load_dynamic_library(char* dll);
void* platform_load_dynamic_function(void* dll, char* funcName);