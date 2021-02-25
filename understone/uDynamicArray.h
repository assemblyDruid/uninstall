/*
   uDynamicArray Limitations
   -------------------------
     - uDynamicArray does not account for functions which modify the `data`
       member directly, as it cannot tell how many elements may have been
       added. To do this safely, the dynamic array must have been set up
       manually or with a call to uDAInit() which allocated to proper number
       of bytes before the direct modification. If this is the case, ask
       yourself: "Do I really need a dynamic array?". If the answer is still
       yes, just remember that uDynamicArray only tracks element-counts and
       resizes properly via functions designed to interact with and modify
       uDynamicArrays, like uDAPush(). You may run into issues with assertions
       and sanity checks within the uDynamicArray functions if you attempt to
       use them after altering the `data` member yourself.
*/

#ifndef __uDynamicArray__
#define __uDynamicArray__ 1

#include "debug_tools.h"
#include "type_tools.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Optimize for random access, "push back", "pop last",
// capping out at 2^16 elements via IEEE size_t.
typedef struct
{
    void* volatile data;

    const size_t          datatype_size;
    volatile const size_t length;
    volatile const size_t scaling_factor;
    volatile const size_t max_length;
} uDynamicArray;

#define uDAInit(type) uAPI_uDAInit(sizeof(type))
static uDynamicArray*
uAPI_uDAInit(const size_t datatypesize_in)
{
    uAssertMsg_v(datatypesize_in, "Data type size must be non-zero.\n");
    uDynamicArray* const da = (uDynamicArray*)calloc(1, sizeof(uDynamicArray));

    // Initialize statics
    size_t* non_const_length         = (size_t*)&(da->length);
    size_t* non_const_scaling_factor = (size_t*)&(da->scaling_factor);
    size_t* non_const_max_length     = (size_t*)&(da->max_length);
    size_t* non_const_datatype_size  = (size_t*)&(da->datatype_size);

    *non_const_length         = 0;
    *non_const_scaling_factor = 2;
    *non_const_max_length     = 2;
    *non_const_datatype_size  = datatypesize_in;

    // Initialize Dynamics
    da->data = (void*)malloc(*non_const_datatype_size * da->max_length);

    return da;
}

#define uDAPush(da, data_in) uAPI_uDAPush(da, VPPC_STR_LITERAL(void**) data_in)
static bool
uAPI_uDAPush(uDynamicArray* const restrict da, void** const restrict data_in)
{
    uAssertMsg_v(da, "Null uDynamicArray ptr provided.\n");
    uAssertMsg_v(data_in, "Null data ptr pvoided.\n");

    if (da && data_in)
    {
        if (da->length >= da->max_length)
        {
            size_t* non_const_max_length = (size_t*)&(da->max_length);
            *non_const_max_length        = da->max_length * da->scaling_factor;
            void* allocated              = realloc(da->data, (da->datatype_size * da->max_length));
            uAssertMsg_v(allocated, "[ dynamic array ] Reallocation failed.\n");
            if (allocated)
            {
                da->data = allocated;
            }
        }

        memcpy((char*)da->data + (da->length * da->datatype_size), data_in, da->datatype_size);

        size_t* non_const_length = (size_t*)&(da->length);
        *non_const_length        = *non_const_length + 1;

        return true;
    }

    return false;
}

static void*
uDAIndex(uDynamicArray* const restrict da, const size_t index)
{
    uAssertMsg_v(da, "Null uDynamicArray ptr pvoided.\n");
    uAssertMsg_v(index < da->length,
                 "Index [ %zd ] surpasses dynamic array length: [ %zd ].\n",
                 index,
                 da->length);

    if (da && (index < da->length))
    {
        return (void*)((char*)da->data + (index * da->datatype_size));
    }

    return NULL;
}

static bool
uDAPop(uDynamicArray* const restrict da)
{
    uAssertMsg_v(da, "Null uDynamicArray ptr pvoided.\n");
    uAssertMsg_v(da->length, "Dynamic array length must be non-zero\n");
    uAssertMsg_v(da->length <= da->max_length,
                 "Dynamic array length must be less than the maximum length");

    if (da && da->length && da->length <= da->max_length)
    {
        size_t* non_const_length = (size_t*)&(da->length);
        *non_const_length        = (*non_const_length) - 1;

        return true;
    }

    return false;
}

// [ cfarvin::RESTORE ] Unused fn warning
/* static bool */
/* uDAFitToSize( uDynamicArray* const restrict da) */
/* { */
/*     uAssertMsg_v(da, "Null uDynamicArray ptr provided.\n"); */

/*     if (da && (da->length > da->scaling_factor)) */
/*     { */
/*         size_t* non_const_max_length = (size_t*) &(da->max_length); */
/*         *non_const_max_length = da->length; */
/*         void* realloc_return = realloc(da->data, (da->datatype_size *
 * da->max_length)); */
/*         if (realloc_return) */
/*         { */
/*             da->data = realloc_return; */
/*             return true; */
/*         } */
/*     } */

/*     return false; */
/* } */

// [ cfarvin::RESTORE ] Unused fn warning
/* static bool */
/* uDASetScalingFactor( uDynamicArray* const restrict da, */
/*                     const size_t scaling_factor_in) */
/* { */
/*     uAssertMsg_v(da, "Null uDynamicArray ptr provided.\n"); */
/*     uAssertMsg_v(scaling_factor_in > 1, */
/*                  "Scaling factor must be greater than 1.\n"); */

/*     if (da && scaling_factor_in > 1) */
/*     { */
/*         size_t* non_const_scaling_factor = (size_t*) &da->scaling_factor; */
/*         *non_const_scaling_factor = scaling_factor_in; */

/*         return true; */
/*     } */

/*     return false; */
/* } */

static bool
uDADestroy(uDynamicArray* const restrict da)
{
    if (da && da->data)
    {
        free(da->data);
        free(da);

        return true;
    }

    return false;
}

#endif // __uDynamicArray
