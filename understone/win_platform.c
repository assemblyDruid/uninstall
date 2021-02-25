// [ cfarvin::TODO ] Curate headers

#include "win_platform.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

/* #include "macro_tools.h" */
#include "debug_tools.h"
#include "engine_info.h"
#include "event_tools.h"
#include "type_tools.h"
#include "window_tools.h"

bool
uCreateWin32Info(uGameWindowProperties* const restrict game_window_props,
                 LRESULT CALLBACK (*WindowProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam),
                 uWin32Info* const restrict return_win32_info)
{
    uAssertMsg_v(NULL != game_window_props,
                 "[ win32 ] uGameWindowProperties ptr must be non null.\n");
    uAssertMsg_v(NULL != return_win32_info, "[ win32 ] uWin32Info ptr must be non null.\n");

    const char* const window_class_name = uGetEngineName();
    return_win32_info->command_show     = 10;

    WNDCLASSEX window_class  = { 0 };
    window_class.style       = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = WindowProc;
    window_class.hInstance = return_win32_info->instance = GetModuleHandle(NULL);
    window_class.lpszClassName = return_win32_info->class_name = window_class_name;
    window_class.cbSize                                        = sizeof(WNDCLASSEX);
    /*
      window_class.hIcon = NULL;
      window_class.hCursor = NULL;
      window_class.hbrBackground = NULL;
      window_class.lpszMenuName = NULL;
      window_class.hIconSm = NULL;
    */

    if (false == RegisterClassEx(&window_class))
    {
        uError("[ win32 ] Could not register window class. Last error code: %lu\n", GetLastError());
        return false;
    }

    return_win32_info->window = CreateWindowEx(0,
                                               window_class.lpszClassName,
                                               (const char*)window_class_name,
                                               WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                               CW_USEDEFAULT,
                                               CW_USEDEFAULT,
                                               CW_USEDEFAULT,
                                               CW_USEDEFAULT,
                                               0,
                                               0,
                                               return_win32_info->instance,
                                               0);

    if (NULL == return_win32_info->window)
    {
        uError("Windows returned null handle to client window.\n");
        return false;
    }

    if (false == IsWindow(return_win32_info->window))
    {
        uError("Windows reports that the window is invalid.\n");
        return false;
    }

    ShowWindow(return_win32_info->window, return_win32_info->command_show);
    game_window_props->is_minimized = false;

    return true;
}

void
uWin32ProcessSystemEvents()
{
    MSG msg = { 0 };
    PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
