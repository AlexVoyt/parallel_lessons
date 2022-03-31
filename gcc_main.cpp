#include <omp.h>
#include <cstdlib>
#include <thread>
#include <vector>
#include <barrier>
#include <type_traits>
#include <stdint.h>
#include <assert.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#include "functions.cpp"
#include "thread_utils.cpp"

namespace std {
    constexpr std::size_t hardware_constructive_interference_size = 64u;
    constexpr std::size_t hardware_destructive_interference_size = 64u;
}

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
    void* Result = aligned_alloc(Alignment, Size);
    assert(Result);

    return Result;
}

void FreeAlign(void* Memory)
{
    free(Memory);
}

// Returns cache line size in bytes
constexpr u32 GetCacheLineSize()
{
    return std::hardware_destructive_interference_size;
}

#include "integration_cpp.cpp"
#include "integration_omp.cpp"
#include "reduction.cpp"
#include "random_generator.cpp"
#include "fibonacci.cpp"
#include "experiment.cpp"


int main()
{
#if 1
#if 0
    SHOW_INTEGRATION_EXPERIMENT(IntegrateMutex);
    SHOW_INTEGRATION_EXPERIMENT(IntegratePS);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateAtomic);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateReduction);

    SHOW_INTEGRATION_EXPERIMENT(IntegrateAlignOMP);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateParallelOMP); // omp critical
    SHOW_INTEGRATION_EXPERIMENT(IntegrateFalseSharingOMP);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateAtomicOMP);
    SHOW_INTEGRATION_EXPERIMENT(IntegrateReductionOMP);
#endif

    u32 Thousand = 1000;
    u32 Million = Thousand * Thousand;
    const u32 ArrayLength = 50 * Million;

    u32* SingleThreadedArray = (u32* )Allocate(sizeof(u32)*ArrayLength);
    u32* FalseSharingArray = (u32* )Allocate(sizeof(u32)*ArrayLength);
    u32* MultiThreadedArray = (u32* )AllocateAlign(sizeof(u32)*ArrayLength, GetCacheLineSize());

#if 0
    SHOW_RANDOMIZATION_EXPERIMENT(RandomizeArraySingleThreaded, SingleThreadedArray, ArrayLength);
#else
    printf("%s (%d elements)\n", "RandomizeArraySingleThreaded", ArrayLength);
    SetThreadCount(1);

    u32 Width = 4;
    printf("%-*s %-*s %-*s %-*s %-*s %-*s\n",
            (int)(strlen("Threads") + Width),           "Threads",
            (int)(strlen("Expected Average") + Width),  "Expected Average",
            (int)(strlen("Average") + Width),           "Average",
            (int)(strlen("DifferenceAverage") + Width), "DifferenceAverage",
            (int)(strlen("Time(s)") + Width),           "Time(s)",
            (int)(strlen("Acceleration") + Width),      "Acceleration");

    randomization_experiment_result Experiment;
    Experiment = RunExperiment(RandomizeArraySingleThreaded, SingleThreadedArray, ArrayLength);
    printf("%-*u %-*f %-*f %-*f %-*f %-*f\n",
            (int)(strlen("Threads") + Width),           1,
            (int)(strlen("Expected Average") + Width),  Experiment.ExpectedAverage,
            (int)(strlen("Average") + Width),           Experiment.Average,
            (int)(strlen("DifferenceAverage") + Width), Experiment.DifferenceAverage,
            (int)(strlen("Time(s)") + Width),           Experiment.Time,
            (int)(strlen("Acceleration") + Width),      1.0);
#endif

    SHOW_RANDOMIZATION_EXPERIMENT(RandomizeArray, MultiThreadedArray, ArrayLength);
    SHOW_RANDOMIZATION_EXPERIMENT(RandomizeArrayFalseSharing, FalseSharingArray, ArrayLength);
#endif

    SHOW_FIBONACCI_EXPERIMENT(Fibonacci);


#if 1
    // Testing
    for(u32 Index = 0; Index < ArrayLength; Index++)
    {
        assert(SingleThreadedArray[Index] == MultiThreadedArray[Index]);
        assert(MultiThreadedArray[Index] == FalseSharingArray[Index]);
    }
#endif

    printf("\n");
    return 0;
}

