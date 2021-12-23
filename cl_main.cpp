#include <omp.h>
#include <cstdlib>
#include <thread>
#include <vector>
#include <barrier>
#include <type_traits>
#include <assert.h>

#include "functions.cpp"
#include "thread_utils.cpp"

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

#include "integration_cpp.cpp"
#include "integration_omp.cpp"
#include "reduction.cpp"
#include "experiment.cpp"


int main()
{
#if 1
    SHOW_EXPERIMENT(IntegratePS);
    SHOW_EXPERIMENT(IntegrateReductionOMP);
    SHOW_EXPERIMENT(IntegrateFalseSharingOMP);
    SHOW_EXPERIMENT(IntegrateParallelOMP);
    SHOW_EXPERIMENT(IntegrateAlignOMP);
#endif
    SHOW_EXPERIMENT(IntegrateReduction);

    return 0;
}


