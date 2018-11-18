#include <engine_tools/type_tools.h>
#include <engine_tools/event_tools.h>
#include <engine_tools/ogl_tools.h>
#include <engine_tools/stats_tools.h>

#if __linux__
#include <nix/nix_platform.h>
#elif _WIN32
#include <win/win_platform.h>
#endif // _WIN32

//
// renderers
//
#include <renderers/triangle_renderer.h>

uint8 RUNNING = 1;

void
uHandleWindowResize()
{
    printf("uHandleWindowResize()\n");
}

void
uRefreshInputState()
{
#if __linux__
    uEVENT event = x11_handleEvents();
#elif _WIN32
    uEVENT event = uWin32HandleEvents();
#endif // __linux__ _WIN32

    stats.events_handled_this_loop = 0;

    switch(event)
    {
        case uEVENT_NONE:
        {
            break;
        }
        case uEVENT_RESIZE:
        {
            glViewport(0,
                       0,
                       viewport.width,
                       viewport.height);
            break;
        }
        case uEVENT_CLOSE:
        {
            RUNNING = 0;
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_PRESS_LEFT:
        {
            printf("ButtonPress | Mouse Left\n");
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_PRESS_RIGHT:
        {
            printf("ButtonPress | Mouse Right\n");
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_PRESS_MIDDLE:
        {
            printf("ButtonPress | Mouse Middle\n");
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_RELEASE_LEFT:
        {
            printf("ButtonRelease | Mouse Left\n");
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_RELEASE_RIGHT:
        {
            printf("ButtonRelease | Mouse Right\n");
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_RELEASE_MIDDLE:
        {
            printf("ButtonRelease | Mouse Middle\n");
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_SCROLL_UP:
        {
            printf("ButtonPress | Mouse Scroll Up\n");
            stats.events_handled_this_loop++;
            break;
        }
        case uEVENT_MOUSE_SCROLL_DOWN:
        {
            printf("ButtonPress | Mouse Scroll Down\n");
            stats.events_handled_this_loop++;
            break;
        }

        default:
        {
            return;
        }
    }
}

void
uInitializeGameWindowsAndContext()
{
#if __linux__
    uX11CreateWindow();
#endif // __linux__

#if _WIN32
    uWin32CreateWindow();
#endif // _WIN32

    uRefreshInputState();
}

void
uInitializeRenderers()
{
    // [ cfarvin::REMOVE ] linux #if
#if __linux__
    uInitRenderer(triangle); // should work like this;
#endif // __linux__
}

inline void
uSwapBuffers()
{
#if __linux__
    glXSwapBuffers(x11.display, x11.engine_window);
#eleif _WIN32
    SwapBuffers(WIN32_INFO.device_context);
#endif
}

void
uDestroyEngine()
{
    printf("[ DESTROY ENGINE ]\n");
#if __linux__
    uX11Destroy();
#endif // __linux__
    // [ cfarvin::TODO ] destroy eingine functionality for win32

}

#if _WIN32
int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow)
#else
    int
    main(int argc, char** argv)
#endif // _WIN32
{
#if _WIN32
    win32.instance = hInstance;
    win32.command_show = nCmdShow;
    win32.class_name  = "UE Window Class";
#endif // _WIN32

    uInitializeGameWindowsAndContext();
    /* initializeRenderers(); */

    while(RUNNING)
    {
        /* glError; */

        uRefreshInputState();

        /* glError; */
        /* /\* render_triangle(); *\/ */
        /* glError; */

        /* glError; */
        uSwapBuffers();
        /* glError; */
    }

    uDestroyEngine();

    printf("[ SUCCESS ]\n");
    return 0;
}
