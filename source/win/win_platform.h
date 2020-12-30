#ifndef __win_platform__
#define __win_platform__ 1

#ifndef WIN32
#define WIN32
#endif // WIN32
#include "data_structures.h"
#include "debug_tools.h"
#include "engine_info.h"
#include "event_tools.h"
#include "macro_tools.h"
#include "type_tools.h"
#include "window_tools.h"

#include <stdint.h>
#include <stdio.h>
#include <windows.h>

uSystemEvent kWin32SystemEvent      = {};
POINT        kWin32MouseCoordinates = {};

extern uGameWindow kGameWindow;

//
// [ begin ] Prime uWin32Info
typedef struct
{
    HINSTANCE instance;
    int       command_show;
    HWND      window;
    HDC       device_context;
    const s8* class_name;
} uWin32Info;
const uWin32Info* uAPI_PRIME_WIN32_INFO = NULL;

// Forward declare creation method
static const uWin32Info*
uWin32CreateWin32Info();

__UE_inline__ static const uWin32Info*
uGetWin32Info()
{
    if(!uAPI_PRIME_WIN32_INFO) { *( uWin32Info** )&uAPI_PRIME_WIN32_INFO = ( uWin32Info* )uWin32CreateWin32Info(); }

    return uAPI_PRIME_WIN32_INFO;
}
// [ end ] Prime uWin32Info
//

// [ cfarvin::REMOVE ] There is a lot of "RESTORE" tags, I am worried that removing this now will end up
//                     doing away with something I needed for one of them. Finish those first, then remove
//                     this if necessary.
/* static __UE_inline__ void */
/* uWin32GetWindowSize(u32* const restrict width, u32* const restrict height) */
/* { */
/*     const uWin32Info* win32_info = uGetWin32Info(); */

/*     uAssertMsg_v(width, "[ win32 ] Width ptr must be non null.\n"); */
/*     uAssertMsg_v(height, "[ win32 ] Height ptr must be non null.\n"); */
/*     uAssertMsg_v(IsWindow(win32_info->window), */
/*                  "[ win32 ] Windows reports that the win32_info->window member " */
/*                  "is invalid.\n"); */

/*     RECT window_rect        = {}; */
/*     BOOL win32_rect_success = GetWindowRect(win32_info->window, &window_rect); */
/*     if(!win32_rect_success) */
/*     { */
/*         uFatal("[ win32 ] Unable to determine window rect with win32 error: %lu.\n", GetLastError()); */
/*     } */

/*     uAssert(window_rect.right > window_rect.left); */
/*     uAssert(window_rect.bottom > window_rect.top); */

/*     *width  = window_rect.right - window_rect.left; */
/*     *height = window_rect.bottom - window_rect.top; */

/*     uAssert(*width); */
/*     uAssert(*height); */
/* } */

LRESULT CALLBACK
uEngineWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CLOSE:
        {
            kWin32SystemEvent = uEventClose;
            break;
        }

        case WM_DESTROY:
        {
            kWin32SystemEvent = uEventClose;
            PostQuitMessage(0);
            break;
        }

        case WM_LBUTTONDOWN:
        {
            uSetInputPressed(uMouse_left);
            break;
        }

        case WM_RBUTTONDOWN:
        {
            uSetInputPressed(uMouse_right);
            break;
        }

        case WM_LBUTTONUP:
        {
            uSetInputReleased(uMouse_left);
            break;
        }

        case WM_RBUTTONUP:
        {
            uSetInputReleased(uMouse_right);
            break;
        }

        case WM_SIZE:
        {
            // [ cfarvin::TODO ] scaling/ortho
            kGameWindow.width  = ( u16 )LOWORD(lParam);
            kGameWindow.height = ( u16 )HIWORD(lParam);
            kWin32SystemEvent  = uEventResize;

            if(wParam == SIZE_MINIMIZED) { kGameWindow.is_minimized = true; }
            else if(wParam == SIZE_RESTORED)
            {
                kGameWindow.is_minimized = false;
            }

            break;
        }

        case WM_MOUSEMOVE:
        {
            GetCursorPos(&kWin32MouseCoordinates);
            ScreenToClient(hwnd, &kWin32MouseCoordinates);

            // Note: uMousePos has origin @ lower left == (0, 0, 0)
            mouse_pos.x = ( u16 )kWin32MouseCoordinates.x;
            mouse_pos.y = (u16)(kGameWindow.height - kWin32MouseCoordinates.y);
            break;
        }
    }

    // Default Window Procedure
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static const uWin32Info*
uWin32CreateWin32Info()
{
    if(uAPI_PRIME_WIN32_INFO) { return uAPI_PRIME_WIN32_INFO; }

    const s8* window_class_name = uGetEngineName();

    *( uWin32Info** )&uAPI_PRIME_WIN32_INFO                 = ( uWin32Info* )calloc(1, sizeof(uWin32Info));
    (*( uWin32Info** )&uAPI_PRIME_WIN32_INFO)->class_name   = window_class_name;
    (*( uWin32Info** )&uAPI_PRIME_WIN32_INFO)->instance     = GetModuleHandle(NULL);
    (*( uWin32Info** )&uAPI_PRIME_WIN32_INFO)->command_show = 10;

    WNDCLASSEX window_class    = {};
    window_class.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc   = uEngineWindowProc;
    window_class.hInstance     = uAPI_PRIME_WIN32_INFO->instance;
    window_class.lpszClassName = ( const char* )uAPI_PRIME_WIN32_INFO->class_name;
    window_class.cbSize        = sizeof(WNDCLASSEX);
    /*
      window_class.hIcon = NULL;
      window_class.hCursor = NULL;
      window_class.hbrBackground = NULL;
      window_class.lpszMenuName = NULL;
      window_class.hIconSm = NULL;
    */

    if(!RegisterClassEx(&window_class))
    {
        free(*( uWin32Info** )&uAPI_PRIME_WIN32_INFO);
        uFatal("[ win32 ] Could not register window class; last error code: %lu\n", GetLastError());
    }

    (*( uWin32Info** )&uAPI_PRIME_WIN32_INFO)->window = CreateWindowEx(0,
                                                                       window_class.lpszClassName,
                                                                       ( const char* )window_class_name,
                                                                       WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                                                       CW_USEDEFAULT,
                                                                       CW_USEDEFAULT,
                                                                       CW_USEDEFAULT,
                                                                       CW_USEDEFAULT,
                                                                       0,
                                                                       0,
                                                                       uAPI_PRIME_WIN32_INFO->instance,
                                                                       0);

    if(uAPI_PRIME_WIN32_INFO->window == NULL)
    {
        free(*( uWin32Info** )&uAPI_PRIME_WIN32_INFO);
        uFatal("Windows returned null handle to client window.\n");
    }

    if(!IsWindow(uAPI_PRIME_WIN32_INFO->window))
    {
        free(*( uWin32Info** )&uAPI_PRIME_WIN32_INFO);
        uFatal("Windows reports that uAPI_PRIME_WIN32_INFO->window is invalid.\n");
    }

    ShowWindow(uAPI_PRIME_WIN32_INFO->window, uAPI_PRIME_WIN32_INFO->command_show);
    kGameWindow.is_minimized = false;
    return uAPI_PRIME_WIN32_INFO;
}

// [ cfarvin::RESTORE ] Unused fn warning
/* static __UE_inline__ const uWin32Info* */
/* uWin32CreateWindow() */
/* { */
/*     return uWin32CreateWin32Info(); */
/* } */

__UE_inline__ static uSystemEvent
uWin32HandleEvents()
{
    kWin32SystemEvent = uEventNone;
    MSG msg           = {};
    PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return kWin32SystemEvent;
}

static void
uDestroyWin32()
{
    const uWin32Info* win32_info = uGetWin32Info();
    uAssertMsg_v(win32_info, "uWin32Info ptr must not be null.\n");

    if(win32_info) { free(( void* )win32_info); }
}

#endif // __win_platform__
