#ifndef __event_tools__
#define __event_tools__ 1

#include "type_tools.h"

#include <stdio.h>

typedef enum
{
    uEventNone,
    uEventClose,
    uEventResize,
} uSystemEvent;

// Center of the screen is (0, 0, 0).
typedef struct
{
    u16 x;
    u16 y;
} uMousePos;

#define uKEY_A             (1ULL << 0)
#define uKEY_B             (1ULL << 1)
#define uKEY_C             (1ULL << 2)
#define uKEY_D             (1ULL << 3)
#define uKEY_E             (1ULL << 4)
#define uKEY_F             (1ULL << 5)
#define uKEY_G             (1ULL << 6)
#define uKEY_H             (1ULL << 7)
#define uKEY_I             (1ULL << 8)
#define uKEY_J             (1ULL << 9)
#define uKEY_K             (1ULL << 10)
#define uKEY_L             (1ULL << 11)
#define uKEY_M             (1ULL << 12)
#define uKEY_N             (1ULL << 13)
#define uKEY_O             (1ULL << 14)
#define uKEY_P             (1ULL << 15)
#define uKEY_Q             (1ULL << 16)
#define uKEY_R             (1ULL << 17)
#define uKEY_S             (1ULL << 18)
#define uKEY_T             (1ULL << 19)
#define uKEY_U             (1ULL << 20)
#define uKEY_V             (1ULL << 21)
#define uKEY_W             (1ULL << 22)
#define uKEY_X             (1ULL << 23)
#define uKEY_Y             (1ULL << 24)
#define uKEY_Z             (1ULL << 25)
#define uKEY_0             (1ULL << 26)
#define uKEY_1             (1ULL << 27)
#define uKEY_2             (1ULL << 28)
#define uKEY_3             (1ULL << 29)
#define uKEY_4             (1ULL << 30)
#define uKEY_5             (1ULL << 31)
#define uKEY_6             (1ULL << 32)
#define uKEY_7             (1ULL << 33)
#define uKEY_8             (1ULL << 34)
#define uKEY_9             (1ULL << 35)
#define uKey_arrow_up      (1ULL << 36)
#define uKey_arrow_down    (1ULL << 37)
#define uKey_arrow_left    (1ULL << 38)
#define uKey_arrow_right   (1ULL << 39)
#define uKey_shift_left    (1ULL << 40)
#define uKey_shift_right   (1ULL << 41)
#define uKey_control_left  (1ULL << 42)
#define uKey_control_right (1ULL << 43)
#define uMouse_right       (1ULL << 44)
#define uMouse_left        (1ULL << 45)
#define uMouse_middle      (1ULL << 46)

void
uSetInputPressed(const u64 key, u64* const input_state);

void
uSetInputReleased(const u64 key, u64* const input_state);

u64
uGetInputPressed(const u64 key, u64* const input_state);

#endif // __event_tools__
