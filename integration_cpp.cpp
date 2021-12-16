#include <omp.h>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring>

#define STEPS 100000000
#define CACHE_LINE_SIZE 64u

/* Compile time assert macro */
#if 1
#define ASSERT_CONCAT_(a,b) a##b
#define ASSERT_CONCAT(a,b) ASSERT_CONCAT_(a,b)
#define ct_assert(e) enum {ASSERT_CONCAT(asssert_line_, __LINE__) = 1/(!!(e))}

// ct_assert(sizeof(partial_sum) == CACHE_LINE_SIZE);
#endif

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
double IntegratePS(function F, double a, double b)
{
    double Result = 0;
    double dx = (b-a)/STEPS;
    unsigned int T = get_num_threads();
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
