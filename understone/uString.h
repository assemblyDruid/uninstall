#ifndef __uString__
#define __uString__ 1

#include "debug_tools.h"
#include "macro_tools.h"
#include "type_tools.h"

#include <stdbool.h>
#include <string.h> // [ cfarvin::REMOVE ]

typedef struct
{
    char*        data;
    const size_t length;
    const size_t bytes;
} uString;

static size_t
uStringLen(const char* const uStr)
{
    uAssertMsg_v(uStr, "String must be non null.");

    size_t len = 0;
    if (uStr)
    {
        // Avoid while()
        size_t max = ~((size_t)0);
        for (size_t idx = 0; idx < max; idx++)
        {
            if (uStr[idx] == '\0')
            {
                // Do not include null terminator
                // in length count.
                break;
            }

            len++;
        }
    }

    return len;
}

// Verifies that the uString members have reasonable values,
// and that the uString represents a char stream of non-zero
// length.
static bool
uStringVerify(const uString* const uStr)
{
    bool retVal = true;

    // uStr is non null
    if (!uStr)
    {
        retVal = false;
        uError_v("uString was null.\n");
    }

    // uStr->data is non null
    if (!(retVal && uStr->data))
    {
        retVal = false;
        uError_v("uString->data was null.\n");
    }

    // uStr->length is non zero
    if (!(retVal && uStr->length > 0))
    {
        retVal = false;
        uError_v("uString->length is zero.\n");
    }

    // uStr->bytes is non zero
    if (!(retVal && uStr->bytes > 0))
    {
        retVal = false;
        uError_v("uString->bytes is zero.\n");
    }

    // uStr->bytes >= uStr->length
    if (!(retVal && uStr->bytes >= uStr->length))
    {
        retVal = false;
        uError_v("uString->bytes must be <= uString->length.\n");
    }

    if (retVal)
    {
        size_t null_term_pos = 0;
        size_t char_idx      = 0;
        while (char_idx < uStr->length)
        {
            if ((uStr->data)[char_idx] == '\0')
            {
                // Do not include null terminator
                // in length count.
                null_term_pos = char_idx;
                break;
            }

            char_idx++;
        }

        // uStr->length is reasonable
        if (retVal && (char_idx != uStringLen(uStr->data)))
        {
            retVal = false;
            uError_v("uString->length is incorrect.\n");
        }
    }

    return retVal;
}

static uString*
uStringInit(const char* str)
{
    // [ cfarvin::REVISIT ] Strlen correctness, use throughout uString
    if (str)
    {
        uString* uStr                    = (uString*)calloc(1, sizeof(uString));
        size_t*  non_const_length        = (size_t*)&(uStr->length);
        size_t*  non_const_buffer_length = (size_t*)&(uStr->bytes);

        // strlen does not count '\0'
        *non_const_length        = uStringLen(str);
        *non_const_buffer_length = *non_const_length + 1;

        uStr->data = (char*)calloc(1, sizeof(char) * *non_const_buffer_length);
        memcpy(uStr->data, str, *non_const_length);
        (uStr->data)[*non_const_length] = '\0';

        return uStr;
    }

    return NULL;
}

static bool
uStringDestroy(uString* const uStr)
{
    if (uStr && uStr->data)
    {
        free(uStr->data);
        free(uStr);
        return true;
    }

    return false;
}

// [ cfarvin::TODO ] implement uString substring
/* static inline uDynamicArray* */
/* uStringSubstringIndices(uStringArray* const uSA, const char* sub_string) */
/* {} */

#endif // __uString
