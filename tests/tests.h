#ifndef __UE_TESTS_H__
#define __UE_TESTS_H__

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <understone/debug_tools.h>
#include <understone/maths_tools.h>
#include <understone/memory_tools.h>
#include <understone/type_tools.h>
#include <understone/uDynamicArray.h>
#include <understone/uString.h>

// For now, run tests on startup for every debug build,
// Otherwise, don't run.
#if __UE_debug__ == 1
#define __uTESTS_ENABLED__ 1
#endif // __UE_debug__ == 1

#define ARRAY_TST_FAIL_MSG "Failed dynamic array tests\n"
void
RunDynamicArrayTests()
{
    puts("\tRunning dynamic array tests...");

    uDynamicArray* daTest_u8 = uDAInit(u8);
    for (size_t ii = 0; ii < 0xFF; ii++)
    {
        uDAPush(daTest_u8, &ii);
        uTesetAssert((u8)ii == *(u8*)uDAIndex(daTest_u8, ii), ARRAY_TST_FAIL_MSG);
    }
    for (size_t ii = 0xFF; ii > 0; ii--)
    {
        size_t size_before = daTest_u8->length;
        uDAPop(daTest_u8);
        size_t size_after = daTest_u8->length;
        uTesetAssert((size_after == (size_before - 1)), ARRAY_TST_FAIL_MSG);
    }
    uTesetAssert(uDADestroy(daTest_u8), "Failed to deallocate for uDynamicArray u8 test");

    uDynamicArray* daTest_u16 = uDAInit(u16);
    for (size_t ii = 0; ii < 0xFFFF; ii++)
    {
        uDAPush(daTest_u16, &ii);
        uTesetAssert((u16)ii == *(u16*)uDAIndex(daTest_u16, ii), ARRAY_TST_FAIL_MSG);
    }
    for (size_t ii = 0xFFFF; ii > 0; ii--)
    {
        size_t size_before = daTest_u16->length;
        uDAPop(daTest_u16);
        size_t size_after = daTest_u16->length;
        uTesetAssert((size_after == (size_before - 1)), ARRAY_TST_FAIL_MSG);
    }
    uTesetAssert(uDADestroy(daTest_u16), "Failed to deallocate for uDynamicArray u16 test");

    uDynamicArray* daTest_u32 = uDAInit(u32);
    for (size_t ii = 0; ii < 0xFFFF; ii++)
    {
        uDAPush(daTest_u32, &ii);
        uTesetAssert(((u32)ii == *(u32*)uDAIndex(daTest_u32, ii)), ARRAY_TST_FAIL_MSG);
    }
    for (size_t ii = 0xFFFF; ii > 0; ii--)
    {
        size_t size_before = daTest_u32->length;
        uDAPop(daTest_u32);
        size_t size_after = daTest_u32->length;
        uTesetAssert((size_after == (size_before - 1)), ARRAY_TST_FAIL_MSG);
    }
    uTesetAssert(uDADestroy(daTest_u32), "Failed to deallocate for uDynamicArray u32 test");

    uDynamicArray* daTest_u64 = uDAInit(u64);
    for (size_t ii = 0; ii < 0xFFFF; ii++)
    {
        uDAPush(daTest_u64, &ii);
        uTesetAssert(((u64)ii == *(u64*)uDAIndex(daTest_u64, ii)), ARRAY_TST_FAIL_MSG);
    }
    for (size_t ii = 0xFFFF; ii > 0; ii--)
    {
        size_t size_before = daTest_u64->length;
        uDAPop(daTest_u64);
        size_t size_after = daTest_u64->length;
        uTesetAssert((size_after == (size_before - 1)), ARRAY_TST_FAIL_MSG);
    }
    uTesetAssert(uDADestroy(daTest_u64), "Failed to deallocate for uDynamicArray u64 test");

    uDynamicArray* daTest_r32 = uDAInit(r32);
    for (size_t ii = 0; ii < 0xFFFF; ii++)
    {
        r32 value = (r32)(ii - _PI_);
        uDAPush(daTest_r32, (r32*)&value);
        uTesetAssert((value == *(r32*)uDAIndex(daTest_r32, ii)), ARRAY_TST_FAIL_MSG);
    }
    for (size_t ii = 0xFFFF; ii > 0; ii--)
    {
        size_t size_before = daTest_r32->length;
        uDAPop(daTest_r32);
        size_t size_after = daTest_r32->length;
        uTesetAssert((size_after == (size_before - 1)), ARRAY_TST_FAIL_MSG);
    }
    uTesetAssert(uDADestroy(daTest_r32), "Failed to deallocate for uDynamicArray r32 test");

    uDynamicArray* daTest_r64 = uDAInit(r64);
    for (size_t ii = 0; ii < 0xFFFF; ii++)
    {
        r64 value = (r64)ii - _PI_;
        uDAPush(daTest_r64, &value);
        uTesetAssert((value == *(r64*)uDAIndex(daTest_r64, ii)), ARRAY_TST_FAIL_MSG);
    }
    for (size_t ii = 0xFFFF; ii > 0; ii--)
    {
        size_t size_before = daTest_r64->length;
        uDAPop(daTest_r64);
        size_t size_after = daTest_r64->length;
        uTesetAssert((size_after == (size_before - 1)), ARRAY_TST_FAIL_MSG);
    }
    uTesetAssert(uDADestroy(daTest_r64), "Failed to deallocate for uDynamicArray r64 test");
}

#define memoryArenaTestFailMessage "Failed memory arena tests\n"
void
RunMemroyArenaTests()
{
    puts("\tRunning memory arena tests...");

    u16 allocation_size = 0;

    // Test allocation fail on zero size
    uMemoryArena* memory_arena_zero_alloc_test = uMAInit(allocation_size);
    uTesetAssert((memory_arena_zero_alloc_test == NULL), memoryArenaTestFailMessage);
    // Note: We *want* this uMADestroy to fail
    uTesetAssert(!(uMADestroy(memory_arena_zero_alloc_test)),
                 "Failed to return false from uMADestroy on zero size allocation");

    // Test standard allocation/initialization
    allocation_size                     = 512;
    uMemoryArena* memory_arena_ptr_test = uMAInit(allocation_size);
    uTesetAssert((memory_arena_ptr_test->data != NULL), memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_ptr_test->data ==
                  (memory_arena_ptr_test->data + memory_arena_ptr_test->offset)),
                 memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_ptr_test->arena_size == allocation_size),
                 memoryArenaTestFailMessage);
    uTesetAssert(uMADestroy(memory_arena_ptr_test),
                 "Failed to deallocate on uMemoryArena ptr test");

    // Test standard push
    allocation_size                           = 1;
    uMemoryArena* memory_arena_push_data_test = uMAInit(allocation_size);
    uTesetAssert((memory_arena_push_data_test->data != NULL), memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_push_data_test->data ==
                  (memory_arena_push_data_test->data + memory_arena_push_data_test->offset)),
                 memoryArenaTestFailMessage);
    uTesetAssert(memory_arena_push_data_test->arena_size == allocation_size,
                 memoryArenaTestFailMessage);
    u8  standard_test_target        = 5;
    u8* standard_test_target_result = uMAPushData(memory_arena_push_data_test,
                                                  standard_test_target,
                                                  u8);
    uTesetAssert((*standard_test_target_result == 5), memoryArenaTestFailMessage);
    uTesetAssert(uMADestroy(memory_arena_push_data_test),
                 "Failed to destroy arena on standard data push test.\n");

    // Test over-size push
    /* allocation_size = 1; */
    /* uMemoryArena* memory_arena_overflow_test = uMAInit(allocation_size); */
    /* uTesetAssert((memory_arena_overflow_test->data != NULL),
     * memoryArenaTestFailMessage); */
    /* uTesetAssert((memory_arena_overflow_test->data ==
     * (memory_arena_overflow_test->data + */
    /*                                                       memory_arena_overflow_test->offset)),
     */
    /*                 memoryArenaTestFailMessage); */
    /* uTesetAssert(memory_arena_overflow_test->arena_size == allocation_size,
     * memoryArenaTestFailMessage); */
    /* u16 oversize_test_target = 500; */
    /* u16* oversize_test_target_result =
     * uMAPushData(memory_arena_overflow_test, oversize_test_target, u16); */
    /* uTesetAssert((oversize_test_target_result ==
     * (u16*)(memory_arena_overflow_test->data)), */
    /*                 memoryArenaTestFailMessage); */
    /* uTesetAssert(uMADestroy(memory_arena_overflow_test), "Failed to destroy
     * arena on overflow test.\n"); */

    // Test push data (struct) feature
    typedef struct
    {
        int*  iptr;
        int   i;
        float f;
    } uMATestStruct;

    uMATestStruct testStruct = { NULL, 0, 0.0 };

    allocation_size                        = sizeof(testStruct);
    uMemoryArena* memory_arena_struct_test = uMAInit(allocation_size);
    uTesetAssert((memory_arena_struct_test->data != NULL), memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_struct_test->data ==
                  (memory_arena_struct_test->data + memory_arena_struct_test->offset)),
                 memoryArenaTestFailMessage);
    uTesetAssert(memory_arena_struct_test->arena_size == allocation_size,
                 memoryArenaTestFailMessage);
    uMATestStruct* test_struct_result = uMAPushData(memory_arena_struct_test,
                                                    testStruct,
                                                    uMATestStruct);
    uTesetAssert((test_struct_result->iptr == testStruct.iptr), memoryArenaTestFailMessage);
    uTesetAssert((test_struct_result->i == testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((test_struct_result->f == testStruct.f), memoryArenaTestFailMessage);
    uTesetAssert(uMADestroy(memory_arena_struct_test),
                 "Failed to deallocate on uMemoryArena struct test");

    // Test multi-type push
    allocation_size                            = 336;
    uMemoryArena* memory_arena_multi_type_test = uMAInit(allocation_size);
    uTesetAssert((memory_arena_multi_type_test->data != NULL), memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_multi_type_test->data ==
                  (memory_arena_multi_type_test->data + memory_arena_multi_type_test->offset)),
                 memoryArenaTestFailMessage);
    uTesetAssert(memory_arena_multi_type_test->arena_size == allocation_size,
                 memoryArenaTestFailMessage);
    u8*  testStruct_u8  = uMAPushData(memory_arena_multi_type_test, testStruct.i, u8);
    u16* testStruct_u16 = uMAPushData(memory_arena_multi_type_test, testStruct.i, u16);
    u32* testStruct_u32 = uMAPushData(memory_arena_multi_type_test, testStruct.i, u32);
    u64* testStruct_u64 = uMAPushData(memory_arena_multi_type_test, testStruct.i, u64);
    s8*  testStruct_s8  = uMAPushData(memory_arena_multi_type_test, testStruct.i, s8);
    s16* testStruct_s16 = uMAPushData(memory_arena_multi_type_test, testStruct.i, s16);
    s32* testStruct_s32 = uMAPushData(memory_arena_multi_type_test, testStruct.i, s32);
    s64* testStruct_s64 = uMAPushData(memory_arena_multi_type_test, testStruct.i, s64);
    r32* testStruct_r32 = uMAPushData(memory_arena_multi_type_test, testStruct.i, r32);
    r64* testStruct_r64 = uMAPushData(memory_arena_multi_type_test, testStruct.i, r64);
    uTesetAssert((*testStruct_u8 == (u8)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_u16 == (u16)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_u32 == (u32)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_u64 == (u64)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_s8 == (s8)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_s16 == (s16)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_s32 == (s32)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_s64 == (s64)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_r32 == (r32)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert((*testStruct_r64 == (r64)testStruct.i), memoryArenaTestFailMessage);
    uTesetAssert(uMADestroy(memory_arena_multi_type_test),
                 "Failed to deallocate on uMemoryArena multi-type test");

    // Test array push
    uMemoryArena* memory_arena_array_test = uMAInit(256);
    uTesetAssert((memory_arena_array_test), memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_array_test->data), memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_array_test->offset == 0), memoryArenaTestFailMessage);
    uTesetAssert((memory_arena_array_test->arena_size == 256), memoryArenaTestFailMessage);

    u8 test_array[256];
    for (size_t ii = 0; ii < 256; ii++)
    {
        test_array[ii] = (u8)ii;
    }
    u8* test_array_result = uMAPushArray(memory_arena_array_test, &test_array, u8, 256);
    for (size_t ii = 0; ii < 256; ii++)
    {
        uTesetAssert((test_array_result[ii] == test_array[ii] && test_array[ii] == (u8)ii),
                     memoryArenaTestFailMessage);
    }
    uTesetAssert(uMADestroy(memory_arena_array_test),
                 "Failed to deallocate on uMemoryArena array test");
}

void
RunMathsTests()
{
    puts("\tRunning maths tests...");

    //
    // v3 Tests
    //
    v3 v3A      = { 0 };
    v3 v3B      = { 0 };
    v3 v3Result = { 0 };

    // v3Add()
    v3Set(&v3A, 1.0f, 1.0f, 1.0f);
    v3Set(&v3B, 1.0f, 1.0f, 1.0f);
    v3Add(&v3A, &v3B, &v3Result);
    uTesetAssert(v3Result.x == 2 && v3Result.y == 2 && v3Result.z == 2, "Failed v3Add() tests");

    // v3Sub()
    v3Set(&v3A, 1.0f, 1.0f, 1.0f);
    v3Set(&v3B, 1.0f, 1.0f, 1.0f);
    v3Sub(&v3A, &v3B, &v3Result);
    uTesetAssert(v3Result.x == 0 && v3Result.y == 0 && v3Result.z == 0, "Failed v3Sub() tests");

    // v3ScalarMul()
    v3Set(&v3A, 1.0f, 1.0f, 1.0f);
    v3ScalarMul(&v3A, 5.0f, &v3Result);
    uTesetAssert(v3Result.x == 5 && v3Result.y == 5 && v3Result.z == 5,
                 "Failed v3ScalarMul() tests");

    // v3Mag()
    v3Set(&v3A, 1.0f, 1.0f, 1.0f);
    uTesetAssert(v3Mag(&v3A) == (r32)sqrt(3), "Failed v3Mag() tests");
    v3Set(&v3A, 0.0f, 0.0f, 0.0f);
    uTesetAssert(v3Mag(&v3A) == 0, "Failed v3Mag() tests");

    // v3Norm()
    v3Set(&v3A, 1.0f, 2.0f, 3.0f);
    r32 NormMagv3Result = v3Mag(&v3A);
    uTesetAssert(NormMagv3Result = (r32)sqrt(14), "Failed v3Norm() tests");
    v3Set(&v3Result,
          (v3A.x / NormMagv3Result),
          (v3A.y / NormMagv3Result),
          (v3A.z / NormMagv3Result));
    v3Norm(&v3A);
    uTesetAssert(v3A.x == v3Result.x, "Failed v3Norm() tests");
    uTesetAssert(v3A.y == v3Result.y, "Failed v3Norm() tests");
    uTesetAssert(v3A.z == v3Result.z, "Failed v3Norm() tests");

    // v3IsNorm(), v3SetAndNorm()
    v3SetAndNorm(&v3A, 1.0f, 2.0f, 3.0f);
    uTesetAssert(v3IsNorm(&v3A), "Failed v3SetAndNorm() tests");

    // v3Dot()
    v3SetAndNorm(&v3A, 1.0f, 2.0f, 3.0f);
    v3SetAndNorm(&v3B, 4.0f, 5.0f, 6.0f);
    r32 v3ScalarA = v3Dot(&v3A, &v3B);
    r32 v3ScalarB = (v3A.x * v3B.x) + (v3A.y * v3B.y) + (v3A.z * v3B.z);
    uTesetAssert(v3ScalarA == v3ScalarB, "Failed v3Dot() tests");

    // v3Cross()
    v3Set(&v3A, 1.0f, 2.0f, 3.0f);
    v3Set(&v3B, 1.0f, 5.0f, 7.0f);
    v3Cross(&v3A, &v3B, &v3Result);
    uTesetAssert(IsWithinTolerance(v3Result.x, -1), "Failed v3Cross() tests");
    uTesetAssert(IsWithinTolerance(v3Result.y, -4), "Failed v3Cross() tests");
    uTesetAssert(IsWithinTolerance(v3Result.z, 3), "Failed v3Cross() tests");

    //
    // v4 Tests
    //
    v4 v4A      = { 0 };
    v4 v4B      = { 0 };
    v4 v4Result = { 0 };

    // v4Add()
    v4Set(&v4A, 1.0f, 1.0f, 1.0f, 1.0f);
    v4Set(&v4B, 1.0f, 1.0f, 1.0f, 1.0f);
    v4Add(&v4A, &v4B, &v4Result);
    uTesetAssert(v4Result.x == 2 && v4Result.y == 2 && v4Result.z == 2 && v4Result.w == 2,
                 "Failed v4Add() tests");

    // v4Sub()
    v4Set(&v4A, 1.0f, 1.0f, 1.0f, 1.0f);
    v4Set(&v4B, 1.0f, 1.0f, 1.0f, 1.0f);
    v4Sub(&v4A, &v4B, &v4Result);
    uTesetAssert(v4Result.x == 0 && v4Result.y == 0 && v4Result.z == 0 && v4Result.w == 0,
                 "Failed v4Sub() tests");

    // v4ScalarMul()
    v4Set(&v4A, 1.0f, 1.0f, 1.0f, 1.0f);
    v4ScalarMul(&v4A, 5.0f, &v4Result);
    uTesetAssert(v4Result.x == 5 && v4Result.y == 5 && v4Result.z == 5 && v4Result.w == 5,
                 "Failed v4ScalarMul() tests");

    // v4Mag()
    v4Set(&v4A, 1.0f, 1.0f, 1.0f, 1.0f);
    uTesetAssert(v4Mag(&v4A) == (r32)sqrt(4), "Failed v4Mag() tests");
    v4Set(&v4A, 0.0f, 0.0f, 0.0f, 0.0f);
    uTesetAssert(v4Mag(&v4A) == 0, "Failed v4Mag() tests");

    // v4Norm()
    v4Set(&v4A, 1.0f, 2.0f, 3.0f, 4.0f);
    r32 NormMagv4Result = v4Mag(&v4A);
    uTesetAssert(IsWithinTolerance(NormMagv4Result, (r32)sqrt(30)), "Failed v4Norm() tests");
    v4Set(&v4Result,
          (v4A.x / NormMagv4Result),
          (v4A.y / NormMagv4Result),
          (v4A.z / NormMagv4Result),
          (v4A.w / NormMagv4Result));
    v4Norm(&v4A);
    uTesetAssert(v4A.x == v4Result.x, "Failed v4Norm() tests");
    uTesetAssert(v4A.y == v4Result.y, "Failed v4Norm() tests");
    uTesetAssert(v4A.z == v4Result.z, "Failed v4Norm() tests");

    // v4IsNorm(), v4SetAndNorm()
    v4SetAndNorm(&v4A, 1.0f, 2.0f, 3.0f, 4.0);
    uTesetAssert(v4IsNorm(&v4A), "Failed v4IsNorm() tests");

    // v4Dot()
    v4SetAndNorm(&v4A, 1.0f, 2.0f, 3.0f, 4.0);
    v4SetAndNorm(&v4B, 4.0f, 5.0f, 6.0f, 4.0);
    r32 v4ScalarA = v4Dot(&v4A, &v4B);
    r32 v4ScalarB = (v4A.x * v4B.x) + (v4A.y * v4B.y) + (v4A.z * v4B.z) + (v4A.w * v4B.w);
    uTesetAssert(v4ScalarA == v4ScalarB, "Failed v4Dot() tests");

    //
    // m3 Tests
    //
    m3 m3A      = { 0 };
    m3 m3Result = { 0 };

    // m3Ident()
    m3Ident(&m3A);
    r32 sum = 0;
    for (uint8_t idx = 0; idx < 9; idx++)
    {
        sum += m3A.arr[idx];
    }
    uTesetAssert(sum == 3, "Failed m3Ident() tests");
    uTesetAssert(m3A.arr2d[0][0] == 1, "Failed m3Ident() tests");
    uTesetAssert(m3A.arr2d[1][1] == 1, "Failed m3Ident() tests");
    uTesetAssert(m3A.arr2d[2][2] == 1, "Failed m3Ident() tests");

    // m3Set(),
    m3Set(&m3A, 0);
    sum = 0;
    for (uint8_t idx = 0; idx < 9; idx++)
    {
        sum += m3A.arr[idx];
    }
    uTesetAssert(sum == 0, "Failed m3Set() tests");

    m3Set(&m3A, 1);
    sum = 0;
    for (uint8_t idx = 0; idx < 9; idx++)
    {
        sum += m3A.arr[idx];
    }
    uTesetAssert(sum == 9, "Failed m3Set() tests");

    // union m3
    m3A.i = (v3) { { 1.5, 2.5, 3.5 } };
    m3A.j = (v3) { { 4.5, 5.5, 6.5 } };
    m3A.k = (v3) { { 7.5, 8.5, 9.5 } };
    v3Add(&m3A.i, &m3A.j, &v3Result);
    uTesetAssert(v3Result.x == 6, "Failed m3 union type tests");
    uTesetAssert(v3Result.y == 8, "Failed m3 union type tests");
    uTesetAssert(v3Result.z == 10, "Failed m3 union type tests");
    uTesetAssert(m3A.arr2d[2][0] == 7.5, "Failed m3 union type tests");
    uTesetAssert(m3A.arr2d[2][1] == 8.5, "Failed m3 union type tests");
    uTesetAssert(m3A.arr2d[2][2] == 9.5, "Failed m3 union type tests");
    uTesetAssert(m3A.arr[0] == 1.5, "Failed m3 union type tests");
    uTesetAssert(m3A.arr[1] == 2.5, "Failed m3 union type tests");
    uTesetAssert(m3A.arr[2] == 3.5, "Failed m3 union type tests");

    // m3Mult()
    m3Ident(&m3A);
    m3 m3B = { 0 };
    m3Mult(&m3A, &m3B, &m3Result);
    sum = 0;
    for (uint8_t idx = 0; idx < 9; idx++)
    {
        sum += 0;
    }
    uTesetAssert(sum == 0, "Failed m3Mult() tests");

    m3A.i = (v3) { { 1, 2, 3 } };
    m3A.j = (v3) { { 4, 5, 6 } };
    m3A.k = (v3) { { 7, 8, 9 } };
    m3B.i = (v3) { { 10, 20, 30 } };
    m3B.j = (v3) { { 40, 50, 60 } };
    m3B.k = (v3) { { 70, 80, 90 } };
    m3Mult(&m3A, &m3B, &m3Result);
    v3Set(&v3Result, 300, 360, 420);
    uTesetAssert(v3IsEqual(&m3Result.i, &v3Result), "Failed m3Mult() tests");
    v3Set(&v3Result, 660, 810, 960);
    uTesetAssert(v3IsEqual(&m3Result.j, &v3Result), "Failed m3Mult() tests");
    v3Set(&v3Result, 1020, 1260, 1500);
    uTesetAssert(v3IsEqual(&m3Result.k, &v3Result), "Failed m3Mult() tests");

    //
    // m4 Tests
    //
    m4 m4A      = { 0 };
    m4 m4Result = { 0 };

    // m4Ident()
    m4Ident(&m4A);
    sum = 0;
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        sum += m4A.arr[idx];
    }
    uTesetAssert(sum == 4, "Failed m4Ident() tests");
    uTesetAssert(m4A.arr2d[0][0] == 1, "Failed m4Ident() tests");
    uTesetAssert(m4A.arr2d[1][1] == 1, "Failed m4Ident() tests");
    uTesetAssert(m4A.arr2d[2][2] == 1, "Failed m4Ident() tests");

    // m4Set(),
    m4Set(&m4A, 0);
    sum = 0;
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        sum += m4A.arr[idx];
    }
    uTesetAssert(sum == 0, "Failed m4ISet() tests");

    m4Set(&m4A, 1);
    sum = 0;
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        sum += m4A.arr[idx];
    }
    uTesetAssert(sum == 16, "Failed m4ISet() tests");

    // union m4
    m4A.i = (v4) { { 1.5, 2.5, 3.5, 4.5 } };
    m4A.j = (v4) { { 4.5, 5.5, 6.5, 7.5 } };
    m4A.k = (v4) { { 7.5, 8.5, 9.5, 1.0 } };
    v4Add(&m4A.i, &m4A.j, &v4Result);
    uTesetAssert(v4Result.x == 6, "Failed m4 union type tests");
    uTesetAssert(v4Result.y == 8, "Failed m4 union type tests");
    uTesetAssert(v4Result.z == 10, "Failed m4 union type tests");
    uTesetAssert(v4Result.w == 12, "Failed m4 union type tests");
    uTesetAssert(m4A.arr2d[2][0] == 7.5, "Failed m4 union type tests");
    uTesetAssert(m4A.arr2d[2][1] == 8.5, "Failed m4 union type tests");
    uTesetAssert(m4A.arr2d[2][2] == 9.5, "Failed m4 union type tests");
    uTesetAssert(m4A.arr2d[2][3] == 1.0, "Failed m4 union type tests");
    uTesetAssert(m4A.arr[0] == 1.5, "Failed m4 union type tests");
    uTesetAssert(m4A.arr[1] == 2.5, "Failed m4 union type tests");
    uTesetAssert(m4A.arr[2] == 3.5, "Failed m4 union type tests");
    uTesetAssert(m4A.arr[3] == 4.5, "Failed m4 union type tests");

    // m4Mult()
    m4Ident(&m4A);
    m4 m4B = { 0 };
    m4Mult(&m4A, &m4B, &m4Result);
    sum = 0;
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        sum += 0;
    }
    uTesetAssert(sum == 0, "Failed m4Mult() tests");

    m4A.i = (v4) { { 1, 2, 3, 9 } };
    m4A.j = (v4) { { 4, 5, 6, 8 } };
    m4A.k = (v4) { { 7, 8, 9, 7 } };
    m4A.n = (v4) { { 10, 11, 12, 6 } };
    m4B.i = (v4) { { 10, 20, 30, 90 } };
    m4B.j = (v4) { { 40, 50, 60, 80 } };
    m4B.k = (v4) { { 70, 80, 90, 70 } };
    m4B.n = (v4) { { 80, 90, 100, 60 } };
    m4Mult(&m4A, &m4B, &m4Result);
    v4Set(&v4Result, 1020, 1170, 1320, 1000);
    uTesetAssert(v4IsEqual(&m4Result.i, &v4Result), "Failed m4Mult() tests");
    v4Set(&v4Result, 1300, 1530, 1760, 1660);
    uTesetAssert(v4IsEqual(&m4Result.j, &v4Result), "Failed m4Mult() tests");
    v4Set(&v4Result, 1580, 1890, 2200, 2320);
    uTesetAssert(v4IsEqual(&m4Result.k, &v4Result), "Failed m4Mult() tests");
    v4Set(&v4Result, 1860, 2250, 2640, 2980);
    uTesetAssert(v4IsEqual(&m4Result.n, &v4Result), "Failed m4Mult() tests");

    //
    // XorShift32 Tests
    //
    u32 PrevXorState = XorShift32State;
    XorShift32State  = 42;
    r32 rand         = 0;
    for (size_t iter = 0; iter < 10000; iter++)
    {
        rand = NormalBoundedXorShift32();
        uTesetAssert(rand >= 0, "Failed XorShift32() tests.");
        uTesetAssert(rand <= 1, "Failed XorShift32() tests.");
    }

    // Reset XorShift32State
    XorShift32State = PrevXorState;
}

void
RunStringTests()
{
    puts("\tRunning uString tests...");

    uString* str = uStringInit("Testing 1, 2, 3");
    uStringVerify(str);
    uTesetAssert(str->length == 15, "Failed uString->length test.");
    uTesetAssert(str->bytes == 16, "Failed uString->bytes test.");
    uTesetAssert((str->data)[str->length] == '\0', "Failed null terminator position test.");
    uStringDestroy(str);
}

void
runAllTests()
{
    puts("[ tests ] Running All Tests...");

    RunDynamicArrayTests();
    RunMemroyArenaTests();
    RunMathsTests();
    RunStringTests();

    puts("[ tests ] All pass");
    fflush(stdout);
}

#endif // __UE_TESTS_H__
