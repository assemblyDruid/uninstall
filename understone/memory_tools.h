#ifndef __memory_tools__
#define __memory_tools__ 1

#include "debug_tools.h"
#include "type_tools.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Optimize for 64 byte cache line size,
// 32bit word alignment
typedef struct
{
    intptr_t* data;
    u16       offset;
    const u16 arena_size;
} uMemoryArena;

uMemoryArena*
uMAInit(const u16 arena_bytes)
{
    if (!arena_bytes)
    {
        return NULL;
    }

    uMemoryArena* memory_arena = (uMemoryArena*)malloc(sizeof(uMemoryArena));
    memory_arena->data         = (intptr_t*)malloc(arena_bytes);
    memory_arena->offset       = 0;
    u16* non_const_arena_size  = (u16*)&(memory_arena->arena_size);
    *non_const_arena_size      = arena_bytes;

    return memory_arena;
}

// [ cfarvin::RESTORE ] Unused fn warning
/* static void* */
/* uMANext(const uMemoryArena* restrict const memory_arena) */
/* { */
/*     if(!(memory_arena && memory_arena->data && memory_arena->arena_size)) */
/*     { */
/*         return NULL; */
/*     } */

/*     return (void*)(memory_arena->data + memory_arena->offset); */
/* } */

// [ cfarvin::RESTORE ] Unused fn warning
/* #define uMAAllocate(arena, type, num_bytes)     \ */
/*     (type*)uMAAllocate_API__(arena, num_bytes) */
/* static void* */
/* uMAAllocate_API__( uMemoryArena* restrict const memory_arena, */
/*                   const u16                          num_bytes) */
/* { */
/*     if (!(num_bytes && memory_arena && memory_arena->data)) */
/*     { */
/*         return NULL; */
/*     } */

/*     void* return_ptr = (memory_arena->data + memory_arena->offset); */
/*     memory_arena->offset += num_bytes; */
/*     return return_ptr; */
/* } */

#define uMAPushData(arena, new_data, type) \
    (type*)uMAPushData_API(arena, (type*)&(new_data), sizeof(type))
void*
uMAPushData_API(uMemoryArena* restrict const memory_arena,
                const void* restrict const   new_data,
                const u16                    new_data_size)
{
    if (!(memory_arena && new_data && new_data_size))
    {
        return NULL;
    }

    if ((memory_arena->offset + new_data_size) > (memory_arena->arena_size))
    {
        uError("[ DATA ] Attempted an illegal push to uMemoryArena (overshot arena "
               "size)\n");
        return (memory_arena->data + memory_arena->offset);
    }

    intptr_t* dest_ptr = memory_arena->data + memory_arena->offset;
    memcpy(dest_ptr, new_data, (size_t)new_data_size);
    memory_arena->offset += new_data_size;
    return (void*)dest_ptr;
}

#define uMAPushArray(arena, new_data, type, num_bytes) \
    (type*)uMAPushArray_API(arena, new_data, num_bytes)
void*
uMAPushArray_API(uMemoryArena* memory_arena, void* new_data, u16 new_data_size)
{
    if (!(memory_arena && new_data && new_data_size))
    {
        return NULL;
    }

    if ((memory_arena->offset + new_data_size) > (memory_arena->arena_size))
    {
        uError("[ ARRAY ] Attempted an illegal push to uMemoryArena (overshot "
               "arena size)\n");
        return (memory_arena->data + memory_arena->offset);
    }

    intptr_t* dest_ptr = memory_arena->data + memory_arena->offset;
    memcpy(dest_ptr, new_data, (size_t)new_data_size);
    memory_arena->offset += new_data_size;
    return (void*)(dest_ptr);
}

bool
uMADestroy(uMemoryArena* memory_arena)
{
    if (!(memory_arena && memory_arena->data))
    {
        return false;
    }

    free(memory_arena->data);
    free(memory_arena);
    return true;
}

#endif // __memory_tools
