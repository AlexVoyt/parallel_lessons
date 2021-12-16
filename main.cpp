#include <omp.h>
#include <cstdlib>
#include <thread>
#include <vector>
#include <barrier>
#include <type_traits>

#include <new>

/* Compile time assert macro */
#define ASSERT_CONCAT_(a,b) a##b
#define ASSERT_CONCAT(a,b) ASSERT_CONCAT_(a,b)
#define ct_assert(e) enum {ASSERT_CONCAT(asssert_line_, __LINE__) = 1/(!!(e))}

#if defined(__GNUC__)
namespace std {
    constexpr std::size_t hardware_constructive_interference_size = 64u;
    constexpr std::size_t hardware_destructive_interference_size = 64u;
}
#endif

#include <iostream>

#include "functions.cpp"
#include "thread_utils.cpp"

struct partial_sum
{
    alignas(64) double Value;
};

#include "integration_cpp.cpp"
#include "integration_omp.cpp"
#include "reduction.cpp"

struct experiment_result
{
    double Result;
    double TimeMS;
};

typedef double (*integrate_function) (unary_function, double, double);
experiment_result RunExperiment(integrate_function I)
{
    double t0 = omp_get_wtime();
    double res = I(Quadratic, -1, 1);
    double t1 = omp_get_wtime();

    experiment_result Result;
    Result.Result = res;
    Result.TimeMS = t1 - t0;

    return Result;
}

void ShowExperimentResult(integrate_function I, const char* Name)
{
    printf("%s\n", Name);
    SetThreadCount(1);

    printf("%10s, %10s %10sms %14s\n", "Threads", "Result", "TimeMS", "Acceleration");
    experiment_result Experiment;
    Experiment = RunExperiment(I);
    printf("%10d, %10g %10gms %14g\n", 1, Experiment.Result, Experiment.TimeMS, 1.0f);
    double Time = Experiment.TimeMS; // TODO: rename

    for(unsigned T = 2; T <=omp_get_num_procs(); T++)
    {
        SetThreadCount(T);
        Experiment = RunExperiment(I);
        printf("%10d, %10g %10gms %14g\n", T, Experiment.Result, Experiment.TimeMS, Time/Experiment.TimeMS);
    }
    printf("\n");
}

#define SHOW_EXPERIMENT(fun) ShowExperimentResult((fun), (#fun))

int main()
{
#if 1
    SetThreadCount(4);
    unsigned V[16];
    for(unsigned i = 0; i < std::size(V); i++)
        V[i] = i + 1;
     // std::cout << "as Average: " << '\n';
     std::cout << "Average: " << reduce_vector(V, std::size(V), [](auto x, auto y) {return x + y;}, 0u)/ std::size(V) << '\n';
     // std::cout << "Average: " << reduce_range(1, 16, 10000, Linear, [](auto x, auto y) {return x + y;}, 0) << '\n';
     // std::cout << "sdakfj: " << IntegrateReduction(-1, 1, Quadratic) << '\n;
#endif

#if 1
    SHOW_EXPERIMENT(IntegratePS);
    SHOW_EXPERIMENT(IntegrateReductionOMP);
    SHOW_EXPERIMENT(IntegrateFalseSharingOMP);
    SHOW_EXPERIMENT(IntegrateParallelOMP);
    SHOW_EXPERIMENT(IntegrateReduction);
#endif

    return 0;
}
