#include <omp.h>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring>

#define STEPS 100000000
#define CACHE_LINE_SIZE 64u

/* Compile time assert macro */
#define ASSERT_CONCAT_(a,b) a##b
#define ASSERT_CONCAT(a,b) ASSERT_CONCAT_(a,b)
#define ct_assert(e) enum {ASSERT_CONCAT(asssert_line_, __LINE__) = 1/(!!(e))}

struct partial_sum
{
    alignas(64) double Value;
};
ct_assert(sizeof(partial_sum) == CACHE_LINE_SIZE);

typedef double (*function)(double);
/*
double IntegrateCppMutex(double a, double b, function F)
{
    using namespace std;
    vector<thread> Threads;
    mutex Mutex;
    unsigned int T = thread::hardware_concurrency();
    double dx = (b-a)/STEPS;
    double Result = 0;

    for(unsigned int t = 0; t < T; t++)
    {
        Threads.emplace_back([=, &Result, &Mutex](){
            double Accum = 0;
            for(unsigned int i = t; i < STEPS; i+=T)
                Accum += Function(dx*i + a);

        }
    }

    {
        unsigned int t = (unsigned int)omp_get_thread_num();
        unsigned int T = (unsigned int)omp_get_num_threads();
        for(unsigned int i = t; i < STEPS; i+=T)
#pragma omp critical
        Result += Accum;
    }

    Result *= dx;
    return Result;
}
*/

#if 1
double Linear(double x)
{
    return 5*x;
}

double Quadratic(double x)
{
    return x*x;
}

double IntegratePS(function F, double a, double b)
{
    double Result = 0;
    double dx = (b-a)/STEPS;
    unsigned int T = std::thread::hardware_concurrency();
    auto Vec = std::vector(T, partial_sum{0.0});
    std::vector<std::thread> Threads;

    auto ThreadProcedure = [dx, T, F, a, &Vec](auto t)
    {
        for(auto i = t; i < STEPS; i+=T)
            Vec[t].Value += F(dx*i + a);
    };

    for(unsigned t = 1; t < T; t++)
        Threads.emplace_back(ThreadProcedure, t);

    ThreadProcedure(0);
    for(auto &Thread : Threads)
        Thread.join();

    for(auto Elem : Vec)
        Result += Elem.Value;

    Result *= dx;
    return Result;
}
#endif

typedef struct experiment_result_
{
    double Result;
    double TimeMS;
} experiment_result;


typedef double (*IntegrateFunction) (function, double, double);
experiment_result RunExperiment(IntegrateFunction I)
{
    double t0 = omp_get_wtime();
    double res = I(Quadratic, -1, 1);
    double t1 = omp_get_wtime();

    experiment_result Result;
    Result.Result = res;
    Result.TimeMS = t1 - t0;

    return Result;
}

void ShowExperimentResult(IntegrateFunction I)
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

double IntegrateAlign(function Function, double a, double b)
{
    unsigned int T;
    double Result = 0;
    double dx = (b-a)/STEPS;
    partial_sum* Accum = 0;

#pragma omp parallel shared(Accum, T)
    {
        unsigned int t = (unsigned int)omp_get_thread_num();
#pragma omp single
        {
            T = (unsigned int)omp_get_num_threads();
            Accum = (partial_sum*) aligned_alloc(CACHE_LINE_SIZE, T*sizeof(*Accum));
            memset(Accum, 0, T*sizeof(*Accum));
        }

        for(unsigned int i = t; i < STEPS; i+=T)
            Accum[t].Value += Function(dx*i + a);
    }

    for(unsigned int i = 0; i < T; i++)
        Result += Accum[i].Value;

    Result *= dx;
    free(Accum);
    return Result;
}


double IntegrateParallel(function Function, double a, double b) 
{
    double Result = 0;
    double dx = (b-a)/STEPS;

#pragma omp parallel
    {
        double Accum = 0;
        unsigned int t = (unsigned int)omp_get_thread_num();
        unsigned int T = (unsigned int)omp_get_num_threads();
        for(unsigned int i = t; i < STEPS; i+=T)
            Accum += Function(dx*i + a);
#pragma omp critical
        Result += Accum;
    }

    Result *= dx;
    return Result;
}

double IntegrateFalseSharing(function Function, double a, double b) 
{
    unsigned int T;
    double Result = 0;
    double dx = (b-a)/STEPS;
    double* Accum = 0;

#pragma omp parallel shared(Accum, T)
    {
        unsigned int t = (unsigned int)omp_get_thread_num();
#pragma omp single
        {
            T = (unsigned int)omp_get_num_threads();
            Accum = (double*) calloc(T, sizeof(double));
        }

        for(unsigned int i = t; i < STEPS; i+=T)
            Accum[t] += Function(dx*i + a);
    }

    for(unsigned int i = 0; i < T; i++)
        Result += Accum[i];

    Result *= dx;
    free(Accum);
    return Result;
}

double IntegrateReduction(function Function, double a, double b)
{
    double Result = 0;
    double dx = (b-a)/STEPS;

#pragma omp parallel for reduction(+:Result)
    for(unsigned int i = 0; i < STEPS; i++)
        Result += Function(dx*i + a);

    Result *= dx;
    return Result;
}

int main(void)
{
#if 0
    experiment_result Result = RunExperiment(IntegratePS);
    printf("Partial sums: Result - %f, TimeMS - %f\n", Result.Result, Result.TimeMS);
#endif 
    printf("IntegratePS\n");
    ShowExperimentResult(IntegratePS);

    printf("IntegrateReduction\n");
    ShowExperimentResult(IntegrateReduction);

    printf("IntegrateAlign\n");
    ShowExperimentResult(IntegrateAlign);

    printf("IntegrateFalseSharing\n");
    ShowExperimentResult(IntegrateFalseSharing);
    return 0;
}
