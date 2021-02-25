#include "event_tools.h"

#include "type_tools.h"

void
uSetInputPressed(const u64 key, u64* const input_state)
{
    *input_state |= key;
}

void
uSetInputReleased(const u64 key, u64* const input_state)
{
    *input_state &= ~key;
}

u64
uGetInputPressed(const u64 key, u64* const input_state)
{
    return *input_state & key;
}
