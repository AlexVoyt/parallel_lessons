#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

double IntegrateAlignOMP(function Function, double a, double b)
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
            // TODO
            Accum = 0; // (partial_sum*) aligned_alloc(CACHE_LINE_SIZE, T*sizeof(*Accum));
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


double IntegrateParallelOMP(function Function, double a, double b)
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

double IntegrateFalseSharingOMP(function Function, double a, double b)
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

double IntegrateReductionOMP(function Function, double a, double b)
{
    double Result = 0;
    double dx = (b-a)/STEPS;

#pragma omp parallel for reduction(+:Result)
    for(int i = 0; i < STEPS; i++)
        Result += Function(dx*i + a);

    Result *= dx;
    return Result;
}
