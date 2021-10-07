#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define STEPS 100000000

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

int main(void)
{
    experiment_result ExpP = RunExperiment(IntegrateParallel);
    experiment_result ExpFalse = RunExperiment(IntegrateParallelWithFalseSharing);
    printf("With local variable: Result - %f, TimeMS - %f\n", ExpP.Result, ExpP.TimeMS);
    printf("With Array: Result - %f, TimeMS - %f\n", ExpFalse.Result, ExpFalse.TimeMS);
}
