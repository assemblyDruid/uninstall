#ifndef __UE_TYPE_TOOLS__
#define __UE_TYPE_TOOLS__

#include "macro_tools.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// vppc = void pointer pointer const
#define VPPC_STR_LITERAL(vppc) (vppc)

//
// Fundamental types
//
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;
typedef float    r32;
typedef double   r64;

typedef struct
{
    size_t return_code;
    char   output[512];
    bool   success;
} uPipeReturnType;

//
// Endian types
//
// [ cfarvin::RESTORE ] Unused
/* static union */
/* { */
/*     s16 dword; */
/*     s8  word[2]; */
/* } uEndiannessUnion; */

/* typedef enum */
/* { */
/*     uLITTLE_ENDIAN = 0, */
/*     uBIG_ENDIAN    = 1 */
/* } uEndianness; */

/* uEndianness */
/* uDetermineSystemEndianness() */
/* { */
/*     uEndiannessUnion.dword = 1; */
/*     if (uEndiannessUnion.word[0]) */
/*     { */
/*         return uLITTLE_ENDIAN; */
/*     } */

/*     return uBIG_ENDIAN; */
/* } */

//
// Size types
//
typedef struct
{
    size_t width;
    size_t height;
    size_t depth;
} uSize;

//
// Color types
//
typedef union
{
    // Little Endian Storage
    struct
    {
        u8 A;
        u8 R;
        u8 G;
        u8 B;
    } channel;

    // Big Endian Storage
    struct
    {
        u8 B;
        u8 G;
        u8 R;
        u8 A;
    } LSB_channel;

    u32 value;
} Color32RGB;

typedef struct
{
    r32 H; // [ 0, 360 ]
    r32 S; // [ 0, 001 ]
    r32 V; // [ 0, 001 ]
} Color32HSV;

#endif // __UE_TYPE_TOOLS__
