#ifndef __UE_MATHS_TOOLS_H___
#define __UE_MATHS_TOOLS_H___

#include "debug_tools.h"
#include "macro_tools.h"
#include "type_tools.h"

#include <float.h>
#include <math.h>
#include <stdbool.h>

static const double MAX_RAY_MAG           = 5.0f;
static const double MIN_RAY_MAG           = 0.0f;
static const double MAX_PPM_HEADER_SIZE   = 25;
static const double MAX_PPM_TRIPPLET_SIZE = 15;
static const double _TOLERANCE_           = 0.00001f;
static const double _PI_                  = 3.1415926535;
static const double _PLANK_CONST_         = 0.000000000000000000000000000000000662607015;
static const double _C_AIR_               = 299700000.0;
static const double _C_VACCUME_           = 299792458.0;

// Begin ignoring clang unused fn warning
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

// [ cfarvin::TODO ] Implement other v2 functions to match v3 and v4
typedef union
{
    struct
    {
        r32 x;
        r32 y;
    };

    r32 arr[2];
} v2;

typedef union
{
    struct
    {
        r32 x;
        r32 y;
        r32 z;
    };

    union
    {
        // Little Endian Storage
        // Access via "channel" macro
        struct
        {
            r32 R;
            r32 G;
            r32 B;
        } MSB_channel;

        // Big Endian Storage
        // Access via "channel" macro
        struct
        {
            r32 B;
            r32 G;
            r32 R;
        } LSB_channel;
    };

    r32 arr[3];
} v3;

typedef union
{
    struct
    {
        r32 x;
        r32 y;
        r32 z;
        r32 w;
    };

    union
    {
        // Little Endian Storage
        // Access via "channel" macro
        struct
        {
            r32 A;
            r32 R;
            r32 G;
            r32 B;
        } MSB_channel;

        // Big Endian Storage
        // Access via "channel" macro
        struct
        {
            r32 B;
            r32 G;
            r32 R;
            r32 A;
        } LSB_channel;
    };

    r32 arr[4];
} v4;

typedef union
{
    struct
    {
        v3 i;
        v3 j;
        v3 k;
    };

    r32 arr[9];
    r32 arr2d[3][3];
} m3;

typedef union
{
    struct
    {
        v4 i;
        v4 j;
        v4 k;
        v4 n;
    };

    r32 arr[16];
    r32 arr2d[4][4];
} m4;

inline static bool
IsWithinTolerance(const r32 value, const r32 target_value)
{
    r32 min = (r32)(value - _TOLERANCE_);
    r32 max = (r32)(value + _TOLERANCE_);
    return ((target_value >= min) && (target_value <= max));
}

// Note: XorShift32State must be initialized to some value
//       before using XorShift32()
static u32 XorShift32State;
inline static u32
XorShift32()
{
    u32 x = XorShift32State;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    XorShift32State = x;
    return x;
}

inline static r32
NormalizeToRange(r32 min_source_range,
                 r32 max_source_range,
                 r32 min_target_range,
                 r32 max_target_range,
                 r32 num_to_normalize)
{
    uAssert(max_source_range > min_source_range);
    uAssert(max_target_range > min_target_range);
    uAssert(max_source_range != min_source_range);
    uAssert(max_target_range != min_target_range);
    uAssert(num_to_normalize >= min_source_range && num_to_normalize <= max_source_range);

    r32 ret = ((r32)(num_to_normalize - min_source_range) /
               (r32)(max_source_range - min_source_range)) *
                (max_target_range - min_target_range) +
              min_target_range;
    return ret;
}

// Note: XorShift32State must be initialized to some value
//       before using NormalBoundedXorShift32()
inline static r32
NormalBoundedXorShift32()
{
    u32 max32 = ~(u32)0;
    r32 rand  = (r32)XorShift32() + (r32)_TOLERANCE_;
    return NormalizeToRange((r32)_TOLERANCE_, (r32)max32, (r32)_TOLERANCE_, 1.0f, rand);
}

inline static r32
NormalRayDistLerp(const r32 old_value)
{
    uAssert(MAX_RAY_MAG > MIN_RAY_MAG);
    return (r32)((old_value - MIN_RAY_MAG) * (1.0f / FLT_MAX));
}

inline static void
DegreesToRadians(const r32 degrees, r32* restrict const result)
{
    uAssert(result);
    *result = (r32)(degrees * (_PI_ / 180.0f));
}

inline static void
RadiansToDegrees(const r32 radians, r32* restrict const result)
{
    uAssert(result);
    *result = (r32)(radians * (180.0f / _PI_));
}

//
// v3
//
inline static void
v3Set(v3* restrict const result, const r32 x, const r32 y, const r32 z)
{
    uAssert(result);
    result->x = x;
    result->y = y;
    result->z = z;
}

inline static bool
v3IsEqual(const v3* restrict const a, const v3* restrict const b)
{
    uAssert(a && b);
    return (IsWithinTolerance(a->x, b->x) && IsWithinTolerance(a->y, b->y) &&
            IsWithinTolerance(a->z, b->z));
}

inline static r32
v3Mag(const v3* restrict const a)
{
    uAssert(a);
    r32 x2   = a->x * a->x;
    r32 y2   = a->y * a->y;
    r32 z2   = a->z * a->z;
    r32 sum2 = x2 + y2 + z2;
    return (r32)sqrt(sum2);
}

inline static bool
v3IsNorm(const v3* restrict const a)
{
    return (IsWithinTolerance(v3Mag(a), 1.0f));
}

inline static void
v3Norm(v3* restrict const a)
{
    uAssert(a);
    r32 magnitude = v3Mag(a);
    uAssert(magnitude);
    if (magnitude)
    {
        a->x /= magnitude;
        a->y /= magnitude;
        a->z /= magnitude;
    }
    else
    {
        v3Set(a, 0.0f, 0.0f, 0.0f);
    }
}

inline static void
v3SetAndNorm(v3* restrict const result, const r32 x, const r32 y, const r32 z)
{
    uAssert(result);
    result->x = x;
    result->y = y;
    result->z = z;
    v3Norm(result);
}

inline static void
v3Add(const v3* restrict const a, const v3* restrict const b, v3* restrict const result)
{
    uAssert(a && b && result);
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;
}

inline static void
v3Sub(const v3* restrict const a, const v3* restrict const b, v3* restrict const result)
{
    uAssert(a && b && result);
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
}

inline static void
v3ScalarMul(const v3* restrict const a, const r32 scalar, v3* restrict const result)
{
    uAssert(a && result);
    result->x = a->x * scalar;
    result->y = a->y * scalar;
    result->z = a->z * scalar;
}

inline static r32
v3Dot(const v3* restrict const a, const v3* restrict const b)
{
    uAssert(a && b);
    r32 result = 0;
    result += a->x * b->x;
    result += a->y * b->y;
    result += a->z * b->z;
    return result;
}

inline static void
v3Cross(const v3* restrict const a, const v3* restrict const b, v3* restrict const result)
{
    uAssert(a && b && result);
    r32 i = ((a->y * b->z) - (a->z * b->y));
    r32 j = ((a->z * b->x) - (a->x * b->z));
    r32 k = ((a->x * b->y) - (a->y * b->x));
    v3Set(result, i, j, k);
}

inline static void
v3CrossAndNorm(const v3* restrict const a, const v3* restrict const b, v3* restrict const result)
{
    uAssert(a && b && result);
    v3Cross(a, b, result);
    v3Norm(result);
}

//
// v4
//
inline static void
v4Set(v4* restrict const result, const r32 x, const r32 y, const r32 z, const r32 w)
{
    uAssert(result);
    result->x = x;
    result->y = y;
    result->z = z;
    result->w = w;
}

inline static bool
v4IsEqual(const v4* restrict const a, const v4* restrict const b)
{
    uAssert(a && b);
    return (IsWithinTolerance(a->x, b->x) && IsWithinTolerance(a->y, b->y) &&
            IsWithinTolerance(a->z, b->z) && IsWithinTolerance(a->w, b->w));
}

inline static r32
v4Mag(const v4* const a)
{
    uAssert(a);
    r32 x2   = a->x * a->x;
    r32 y2   = a->y * a->y;
    r32 z2   = a->z * a->z;
    r32 w2   = a->w * a->w;
    r32 sum2 = x2 + y2 + z2 + w2;
    return (r32)sqrt(sum2);
}

inline static bool
v4IsNorm(const v4* restrict const a)
{
    return (IsWithinTolerance(v4Mag(a), 1.0f));
}

inline static void
v4Norm(v4* restrict const a)
{
    uAssert(a);
    r32 magnitude = v4Mag(a);
    if (magnitude)
    {
        a->x /= magnitude;
        a->y /= magnitude;
        a->z /= magnitude;
        a->w /= magnitude;
    }
    else
    {
        uAssert(!a); // false
        v4Set(a, 0.0f, 0.0f, 0.0f, 0.0f);
    }
}

inline static void
v4SetAndNorm(v4* restrict const result, const r32 x, const r32 y, const r32 z, const r32 w)
{
    uAssert(result);
    result->x = x;
    result->y = y;
    result->z = z;
    result->w = w;
    v4Norm(result);
}

inline static void
v4Add(const v4* restrict const a, const v4* restrict const b, v4* restrict const result)
{
    uAssert(a && b && result);
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;
    result->w = a->w + b->w;
}

inline static void
v4Sub(const v4* restrict const a, const v4* restrict const b, v4* restrict const result)
{
    uAssert(a && b && result);
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
    result->w = a->w - b->w;
}

inline static void
v4ScalarMul(const v4* restrict const a, const r32 scalar, v4* restrict const result)
{
    uAssert(a && result);
    result->x = a->x * scalar;
    result->y = a->y * scalar;
    result->z = a->z * scalar;
    result->w = a->w * scalar;
}

inline static r32
v4Dot(const v4* restrict const a, const v4* restrict const b)
{
    uAssert(a && b);
    r32 result = 0;
    result += a->x * b->x;
    result += a->y * b->y;
    result += a->z * b->z;
    result += a->w * b->w;
    return result;
}

//
// m3
//
static void
m3Set(m3* restrict const a, const r32 b)
{
    uAssert(a);
    for (uint8_t idx = 0; idx < 9; idx++)
    {
        a->arr[idx] = b;
    }
}

static bool
m3IsEqual(const m3* restrict const a, const m3* restrict const b)
{
    uAssert(a && b);
    return ((v3IsEqual(&a->i, &b->i)) && (v3IsEqual(&a->j, &b->j)) && (v3IsEqual(&a->k, &b->k)));
}

static void
m3Ident(m3* restrict const result)
{
    uAssert(result);
    for (uint8_t idx = 0; idx < 9; idx++)
    {
        result->arr[idx] = 0;
    }

    for (uint8_t idx = 0; idx < 3; idx++)
    {
        result->arr2d[idx][idx] = 1;
    }
}

inline static void
m3Mult(const m3* restrict const a, const m3* restrict const b, m3* restrict const result)
{
    uAssert(a && b && result);
    for (uint8_t col = 0; col < 3; col++)
    {
        for (uint8_t row = 0; row < 3; row++)
        {
            result->arr2d[row][col] = a->arr2d[row][0] * b->arr2d[0][col] +
                                      a->arr2d[row][1] * b->arr2d[1][col] +
                                      a->arr2d[row][2] * b->arr2d[2][col];
        }
    }
}

//
// m4
//
static void
m4Set(m4* const a, const r32 b)
{
    uAssert(a);
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        a->arr[idx] = b;
    }
}

static bool
m4IsEqual(const m4* const a, const m4* const b)
{
    uAssert(a && b);
    return ((v4IsEqual(&a->i, &b->i)) && (v4IsEqual(&a->j, &b->j)) && (v4IsEqual(&a->k, &b->k)) &&
            (v4IsEqual(&a->n, &b->n)));
}

static void
m4Ident(m4* const result)
{
    uAssert(result);
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        result->arr[idx] = 0;
    }

    for (uint8_t idx = 0; idx < 4; idx++)
    {
        result->arr2d[idx][idx] = 1;
    }
}

inline static void
m4Mult(const m4* restrict const a, const m4* restrict const b, m4* restrict const result)
{
    uAssert(a && b && result);
    for (uint8_t col = 0; col < 4; col++)
    {
        for (uint8_t row = 0; row < 4; row++)
        {
            result->arr2d[row][col] = a->arr2d[row][0] * b->arr2d[0][col] +
                                      a->arr2d[row][1] * b->arr2d[1][col] +
                                      a->arr2d[row][2] * b->arr2d[2][col] +
                                      a->arr2d[row][3] * b->arr2d[3][col];
        }
    }
}

// End ignoring clang unused fn warning
#pragma clang diagnostic pop
#endif // __UE_MATHS_TOOLS_H___
