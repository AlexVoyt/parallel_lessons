#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>


#define STEPS 100000000
#define CACHE_LINE_SIZE 64u

/* Compile time assert macro */
#define ASSERT_CONCAT_(a,b) a##b
#define ASSERT_CONCAT(a,b) ASSERT_CONCAT_(a,b)
#define ct_assert(e) enum {ASSERT_CONCAT(asssert_line_, __LINE__) = 1/(!!(e))}

typedef struct partial_sum_
{
    double Value;
    double Align[8-1];
} partial_sum;
ct_assert(sizeof(partial_sum) == CACHE_LINE_SIZE);

double Linear(double x)
{
    return 5*x;
}

double Quadratic(double x)
{
    return x*x;
}

typedef double (*function)(double);
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

double IntegrateParallelWithFalseSharing(function Function, double a, double b) 
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
    experiment_result ExpP = RunExperiment(IntegrateParallel);
    printf("With local variable lock: Result - %f, TimeMS - %f\n", ExpP.Result, ExpP.TimeMS);

    experiment_result ExpFalse = RunExperiment(IntegrateParallelWithFalseSharing);
    printf("With Array: Result - %f, TimeMS - %f\n", ExpFalse.Result, ExpFalse.TimeMS);

    experiment_result ExpAlign = RunExperiment(IntegrateAlign);
    printf("With aligned array: Result - %f, TimeMS - %f\n", ExpAlign.Result, ExpAlign.TimeMS);

    experiment_result ExpReduction = RunExperiment(IntegrateReduction);
    printf("With reduction: Result - %f, TimeMS - %f\n", ExpReduction.Result, ExpReduction.TimeMS);
}
