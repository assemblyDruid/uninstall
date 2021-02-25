#ifndef __win_platform__
#define __win_platform__ 1

#include "window_tools.h"

#include <stdbool.h>
#include <windows.h>

typedef struct
{
    HINSTANCE   instance;
    int         command_show;
    HWND        window;
    HDC         device_context;
    const char* class_name;
} uWin32Info;

bool
uCreateWin32Info(uGameWindowProperties* const restrict game_window_props,
                 LRESULT CALLBACK (*WindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam),
                 uWin32Info* const restrict return_win32_info);

void
uWin32ProcessSystemEvents();

#endif // __win_platform__
