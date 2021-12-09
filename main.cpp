#include <omp.h>
#include <cstdlib>
#include <thread>
#include <vector>

/* Compile time assert macro */
#define ASSERT_CONCAT_(a,b) a##b
#define ASSERT_CONCAT(a,b) ASSERT_CONCAT_(a,b)
#define ct_assert(e) enum {ASSERT_CONCAT(asssert_line_, __LINE__) = 1/(!!(e))}

#if defined(__GNUC__) && __GNUC__ <= 10
namespace std {
    constexpr std::size_t hardware_constructive_interference_size = 64u;
    constexpr std::size_t hardware_destructive_interference_size = 64u;
}
#endif

#include <iostream>

#include "functions.cpp"
#include "thread_utils.cpp"
#include "reduction.cpp"

struct partial_sum
{
    alignas(64) double Value;
};

#include "integration_cpp.cpp"
#include "integration_omp.cpp"

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

void ShowExperimentResult(integrate_function I)
{
    printf("%10s, %10s %10sms\n", "Threads", "Result", "TimeMS");
    for(unsigned T = 1; T <=omp_get_num_procs(); T++)
    {
        experiment_result Experiment;
        omp_set_num_threads(T);
        Experiment = RunExperiment(I);
        printf("%10d, %10g %10gms\n", T, Experiment.Result, Experiment.TimeMS);
    }
    printf("\n");
}

int main()
{
    unsigned V[16];
    unsigned U[16];
    unsigned W[16];

    set_num_threads(1);

    for(unsigned i = 0; i < std::size(V); i++)
        V[i] = i + 1;
    std::cout << "Average: " << reduce_vector(V, std::size(V), [](auto x, auto y) {return x + y;}, 0u)/ std::size(V) << '\n';

#if 1
    experiment_result Result = RunExperiment(IntegratePS);
    printf("Partial sums: Result - %f, TimeMS - %f\n", Result.Result, Result.TimeMS);
    printf("IntegratePS\n");
    ShowExperimentResult(IntegratePS);

    printf("IntegrateReduction\n");
    ShowExperimentResult(IntegrateReductionOMP);


#if 0
    printf("IntegrateAlign\n");
    ShowExperimentResult(IntegrateAlignOPM);
#endif

    printf("IntegrateFalseSharing\n");
    ShowExperimentResult(IntegrateFalseSharingOMP);
#endif

    return 0;
}
