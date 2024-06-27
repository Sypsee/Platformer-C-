#include "platform.h"
#include "game_lib.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include "wglext.h"

static HWND window;
static HDC dc;


LRESULT CALLBACK window_callback(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (msg)
    {
        case WM_CLOSE:
        {
            running = false;
            break;
        }

        case WM_SIZE:
        {
            RECT rect = {};
            GetClientRect(window, &rect);
            input->screenSizeX = rect.right - rect.left;
            input->screenSizeY = rect.bottom - rect.top;

            break;
        }

        default:
        {
            result = DefWindowProcA(window, msg, wParam, lParam); // let windows handle other default stuff
        }
    }

    return result;
}

bool platform_create_window(int width, int height, char* title)
{
    HINSTANCE instance = GetModuleHandleA(0);

    WNDCLASSA wc = {};
    wc.hInstance = instance;
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = title;         // not the actual "title" rather the id for our window
    wc.lpfnWndProc = window_callback; // Callback for inputs on the window, DefWindowProcA -> default callback handler

    if (!RegisterClassA(&wc))
    {
        GM_ASSERT(false, "Failed to register window!");
        return false;
    }

    int dwStyle = WS_OVERLAPPEDWINDOW;
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = nullptr;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = nullptr;

    {
        window = CreateWindowExA(
            0,
            title, // again not the actual "title" rather the ref for the lpszClassName from wc
            title, // YES this is the actual title for the window
            dwStyle,
            100,
            100,
            width,
            height,
            NULL,
            NULL,
            instance,
            NULL
        );

        if (window == NULL)
        {
            GM_ASSERT(false, "Failed to create window!");
            return false;
        }

        HDC fakeDC = GetDC(window);
        if (!fakeDC)
        {
            GM_ASSERT(false, "Failed to get device context!");
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;

        int pixelFormat = ChoosePixelFormat(fakeDC, &pfd);
        if (!pixelFormat)
        {
            GM_ASSERT(false, "Failed to choose pixel format!");
            return false;
        }

        if (!SetPixelFormat(fakeDC, pixelFormat, &pfd))
        {
            GM_ASSERT(false, "Failed to set pixel format!");
            return false;
        }

        HGLRC fakeRC = wglCreateContext(fakeDC);
        if (!fakeRC)
        {
            GM_ASSERT(false, "Failed to create Render Context!");
            return false;
        }

        if (!wglMakeCurrent(fakeDC, fakeRC))
        {
            GM_ASSERT(false, "Failed to make current!");
            return false;
        }

        wglChoosePixelFormatARB = 
        (PFNWGLCHOOSEPIXELFORMATARBPROC)platform_load_gl_function("wglChoosePixelFormatARB");
        wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)platform_load_gl_function("wglCreateContextAttribsARB");

        if (!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
        {
            GM_ASSERT(false, "Failed to load OpenGL functions!");
            return false;
        }

        // Clean up time!
        wglMakeCurrent(fakeDC, 0);
        wglDeleteContext(fakeRC);
        ReleaseDC(window, fakeDC);

        DestroyWindow(window);
    }

    // Actual OpenGL initialization
    {
        {
            RECT borderRect = {};
            AdjustWindowRectEx(&borderRect, dwStyle, 0, 0);

            width += borderRect.right - borderRect.left;
            height += borderRect.bottom - borderRect.top;
        }

        window = CreateWindowExA(
            0,
            title, // again not the actual "title" rather the ref for the lpszClassName from wc
            title, // YES this is the actual title for the window
            dwStyle,
            100,
            100,
            width,
            height,
            NULL,
            NULL,
            instance,
            NULL
        );

        if (window == NULL)
        {
            GM_ASSERT(false, "Failed to create window!");
            return false;
        }

        dc = GetDC(window);
        if (!dc)
        {
            GM_ASSERT(false, "Failed to get device context!");
            return false;
        }

        const int pixelAttribs[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
            WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
            WGL_SWAP_METHOD_ARB,    WGL_SWAP_COPY_ARB,
            WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
            WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,     32,
            WGL_ALPHA_BITS_ARB,     8,
            WGL_DEPTH_BITS_ARB,     24,
            0 // Terminate with 0, otherwise OpenGL will throw an Error!
        };

        UINT numPixelFormats;
        int pixelFormat = 0;
        if(!wglChoosePixelFormatARB(dc, pixelAttribs,
                                    0, // Float List
                                    1, // Max Formats
                                    &pixelFormat,
                                    &numPixelFormats))
        {
            GM_ASSERT(0, "Failed to wglChoosePixelFormatARB");
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd = {0};
        DescribePixelFormat(dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        if(!SetPixelFormat(dc, pixelFormat, &pfd))
        {
            GM_ASSERT(0, "Failed to SetPixelFormat");
            return true;
        }

        const int contextAttribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
            0 // Terminate the Array
        };

        HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
        if(!rc)
        {
            GM_ASSERT(0, "Failed to crate Render Context for OpenGL");
            return false;
        }

        if(!wglMakeCurrent(dc, rc))
        {
            GM_ASSERT(0, "Faield to wglMakeCurrent");
            return false;
        }

    }

    ShowWindow(window, SW_SHOW);

    return true;
}

void platform_update_window()
{
    MSG msg;

    while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg); // Calls the callback specified when creating the window
    }
}

void* platform_load_gl_function(char* funcName)
{
    PROC proc = wglGetProcAddress(funcName);
    if (!proc)
    {
        static HMODULE openglDLL = LoadLibraryA("opengl32.dll");
        proc = GetProcAddress(openglDLL, funcName);
        
        if (!proc)
        {
            GM_ASSERT(false, "Failed to load gl functions %s", funcName);
            return nullptr;
        }
    }

    return (void*)proc;
}

void platform_swap_buffers()
{
    SwapBuffers(dc);
}

void* platform_load_dynamic_library(char* dll)
{
    HMODULE result = LoadLibraryA(dll);
    GM_ASSERT(result, "Failed to load dll : %s", dll);

    return result;
}

bool platform_free_dynamic_library(void* dll)
{
    BOOL freeResult = FreeLibrary((HMODULE)dll);
    GM_ASSERT(freeResult, "Failed to Free Library!");

    return (bool)freeResult;
}

void* platform_load_dynamic_function(void* dll, char* funcName)
{
    FARPROC proc = GetProcAddress((HMODULE)dll, funcName);
    GM_ASSERT(proc, "Failed to load function: %s from dll", funcName);

    return (void*)proc;
}