#include <omp.h>
#include <cstdlib>
#include <thread>
#include <new>
#include <vector>
#include <barrier>
#include <type_traits>
#include <assert.h>
#include <stdint.h>

#include "functions.cpp"
#include "thread_utils.cpp"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct partial_sum
{
    alignas(64) double Value;
};

void* Allocate(size_t Size)
{
    void* Result = malloc(Size);
    assert(Result);

    return Result;
}

void Free(void* Memory)
{
    free(Memory);
}

void* AllocateAlign(size_t Size, size_t Alignment)
{
    void* Result = _aligned_malloc(Size, Alignment);
    assert(Result);

    return Result;
}

void FreeAlign(void* Memory)
{
    _aligned_free(Memory);
}

// Returns cache line size in bytes
constexpr u32 GetCacheLineSize()
{
    return std::hardware_destructive_interference_size;
}

#include "integration_cpp.cpp"
#include "integration_omp.cpp"
#include "reduction.cpp"
#include "experiment.cpp"
#include "random_generator.cpp"


int main()
{
#if 1
    SHOW_INTEGRATION_EXPERIMENT(IntegrateMutex);
    SHOW_INTEGRATION_EXPERIMENT(IntegratePS);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateReductionOMP);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateFalseSharingOMP);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateParallelOMP);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateAlignOMP);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateReduction);
#endif

    u32 Thousand = 1000;
    u32 Million = Thousand * Thousand;
    const u32 ArrayLength = 50 * Million;

    // Testing
    u32* SingleThreadedArray = (u32* )Allocate(sizeof(u32)*ArrayLength);
    u32* MultiThreadedArray = (u32* )AllocateAlign(sizeof(u32)*ArrayLength, GetCacheLineSize());

    SHOW_RANDOMIZATION_EXPERIMENT(RandomizeArraySingleThreaded, SingleThreadedArray, ArrayLength);
    SHOW_RANDOMIZATION_EXPERIMENT(RandomizeArray, MultiThreadedArray, ArrayLength);

#if 0
    for(u32 Index = 0; Index < ArrayLength; Index++)
    {
        assert(SingleThreadedArray[Index] == MultiThreadedArray[Index]);
    }
#endif


    printf("\n");

    return 0;
}


