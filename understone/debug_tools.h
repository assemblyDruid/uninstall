#ifndef __UE_DEBUG_TOOLS_H__
#define __UE_DEBUG_TOOLS_H__

#include <stdio.h>
#include <string.h>

#if __UE_debug__ == 1
#define __UE_inline__ /* INLINE REMOVED */
#else                 // __UE_debug__ == 1
#define __UE_inline__ inline
#endif // __UE_debug__ == 1

#ifndef functionFired
#if __UE_debug__ == 1
#define functionFired printf("[ debug ] function fired: %s\n", __func__);
#else
#define functionFired /* Debugging is disabled: functionFired */
#endif                // __UE_debug__ == 1
#endif                // functionFired

/*
 * Understone Error Reporting
 *
 * Error & Warning reporting: (always on)
 *    2. `uFatal` (always verbose): Print a formatted fatal error
 *        with file and line information to stderr and exit the
 *        process with code 666.
 *    2. `uError_v` (verbose): Print a formatted error with file
 *        and line information to stderr (does not halt).
 *    3. `uError`: Print a formatted error without file and line
 *        information to stderr (does not halt).
 *    4. `uWarning`:Print a formatted error without file and line
 *        information to stderr (does not halt).
 *
 * Debug printing: (debug only)
 *    1. `uDebugPrint_v` (verbose): Print a formatted debug string
 *        with file and line information to stderr. (does not
 *        halt).
 *    2. `uDebugPrint`: Print a formatted debug string without
 *        file and line information to stderr. (does not halt).
 *
 * Debug assertions: (debug only)
 *    1. `uAssert_v` (verbose): If the condition is false,
 *        print a message with file and line information to
 *        stderr and exit the process with code 666.
 *    1. `uAssert` (verbose): If the condition is false,
 *        print a message without file and line information to
 *        stderr and exit the process with code 666.
 *
 * Test assertions: (awlays on)
 *    1. `uTestAssert` (always verbose): If the condition is false,
 *        print a message with file and line information to
 *        stderr and exit the process with code 666.
 *
 * Trace printing: (always on)
 *    1. `uTrace` (always verbose): print a message with function
 *        and line information to stdout.
 *
 * The length of the message/error is determined by the
 * `MAX_ERROR_LEN` macro set in the lines below.
 *
 */

#define MAX_ERROR_LEN 512
static char _error_buffer[MAX_ERROR_LEN];
static char _message_buffer[MAX_ERROR_LEN];

//
// Always on
//

// uFatal()
#define uFatal(...)                                        \
    snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__); \
    snprintf(_error_buffer,                                \
             MAX_ERROR_LEN,                                \
             "[ fatal ] %s(%d): %s\n",                     \
             __FILE__,                                     \
             __LINE__,                                     \
             _message_buffer);                             \
    fputs(_error_buffer, stderr);                          \
    fflush(stderr);                                        \
    exit(666)

// uError_v()
#define uError_v(...)                                      \
    snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__); \
    snprintf(_error_buffer,                                \
             MAX_ERROR_LEN,                                \
             "[ error ] %s(%d): %s\n",                     \
             __FILE__,                                     \
             __LINE__,                                     \
             _message_buffer);                             \
    fputs(_error_buffer, stderr);                          \
    fflush(stderr)

// uError()
#define uError(...)                                                          \
    snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__);                   \
    snprintf(_error_buffer, MAX_ERROR_LEN, "[ error ] %s", _message_buffer); \
    fputs(_error_buffer, stderr)

// uWarning()
#define uWarning(...)                                                          \
    snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__);                     \
    snprintf(_error_buffer, MAX_ERROR_LEN, "[ warning ] %s", _message_buffer); \
    fputs(_error_buffer, stderr)

// uTestAssert()
#define uTesetAssert(cond, ...)                                \
    if (!(cond))                                               \
    {                                                          \
        snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__); \
        snprintf(_error_buffer,                                \
                 MAX_ERROR_LEN,                                \
                 "[ assertion ] %s(%d): %s",                   \
                 __FILE__,                                     \
                 __LINE__,                                     \
                 _message_buffer);                             \
        fputs(_error_buffer, stderr);                          \
        fflush(stderr);                                        \
        exit(666);                                             \
    }

// uTrace()
#define uTrace(...)                                        \
    snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__); \
    snprintf(_error_buffer,                                \
             MAX_ERROR_LEN,                                \
             "[ trace ] %s(%d): %s",                       \
             __func__,                                     \
             __LINE__,                                     \
             _message_buffer);                             \
    fputs(_error_buffer, stdout);                          \
    fflush(stdout)

//
// Debug only
//
#if __UE_debug__ == 1

// uDebugPrint_v()
#define uDebugPrint_v(...)                                 \
    snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__); \
    snprintf(_error_buffer,                                \
             MAX_ERROR_LEN,                                \
             "[ debug ] %s(%d): %s",                       \
             __FILE__,                                     \
             __LINE__,                                     \
             _message_buffer);                             \
    fputs(_error_buffer, stderr);                          \
    fflush(stderr)

// uDebugPrint()
#define uDebugPrint(...)                                                     \
    snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__);                   \
    snprintf(_error_buffer, MAX_ERROR_LEN, "[ debug ] %s", _message_buffer); \
    fputs(_error_buffer, stderr);                                            \
    fflush(stderr)

// uAssertMsg()
#define uAssertMsg(cond, ...)                                                        \
    if (!cond)                                                                       \
    {                                                                                \
        snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__);                       \
        snprintf(_error_buffer, MAX_ERROR_LEN, "[ assertion ] %s", _message_buffer); \
        fputs(_error_buffer, stderr);                                                \
        fflush(stderr);                                                              \
        exit(666);                                                                   \
    }

// uAssert_v()
#define uAssertMsg_v(cond, ...)                                \
    if (!(cond))                                               \
    {                                                          \
        snprintf(_message_buffer, MAX_ERROR_LEN, __VA_ARGS__); \
        snprintf(_error_buffer,                                \
                 MAX_ERROR_LEN,                                \
                 "[ assertion ] %s(%d): %s",                   \
                 __FILE__,                                     \
                 __LINE__,                                     \
                 _message_buffer);                             \
        fputs(_error_buffer, stderr);                          \
        fflush(stderr);                                        \
        exit(666);                                             \
    }

// uAssert()
#define uAssert(cond)         \
    if (!((long long)(cond))) \
    {                         \
        exit(666);            \
    }

#define uDebugStatement(statement) statement

#if _WIN32
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif // _WIN32

#else // __UE_debug__ == 1
// Disable the following when not in debug-mode
#define uDebugPrint_v(...)         /* System debugging is disabled */
#define uDebugPrint(...)           /* System debugging is disabled */
#define uAssert(cond, ...)         /* System debugging is disabled */
#define uAssert_v(cond, ...)       /* System debugging is disabled */
#define uAssertMsg_v(cond, ...)    /* System debugging is disabled */
#define uDebugStatement(statement) /* System debugging is disabled */
#endif // __UE_debug__ == 1

#endif // __UE_DEBUG_TOOLS_H__
