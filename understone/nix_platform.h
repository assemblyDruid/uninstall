#ifndef __UE_NIX_PLATFORM_H__
#define __UE_NIX_PLATFORM_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <understone/event_tools.h>
#include <understone/type_tools.h>
#include <understone/uDynamicArray.h>
#include <understone/uString.h>

uSystemEvent
uNixCreateWindow()
{
    uDebugPrint("TODO: Create Window: (X11 -> Weyland)");
    return uEventNone;
}

uSystemEvent
uNixDestroyWindow()
{
    uDebugPrint("TODO: Create Window: (X11 -> Weyland)");
    return uEventNone;
}

uSystemEvent
uNixHandleEvents();

#endif // __UE_NIX_PLATFORM_H__
