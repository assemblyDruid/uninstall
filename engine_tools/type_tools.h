#ifndef __type_tools__
#define __type_tools__ 1

#include <stdint.h>
#include <stddef.h>

#define s8    int8_t
#define s16   int16_t
#define s32   int32_t
#define s64   int64_t
#define u8    uint8_t
#define u16   uint16_t
#define u32   uint32_t
#define u64   uint64_t
#define r32   float
#define r64   double

typedef enum
    {
        false = 0,
        true = 1
    } bool;

typedef struct
{
    size_t width;
    size_t height;
    size_t depth;
} UE_SIZE;

#endif // __type_tools
