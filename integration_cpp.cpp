#include <omp.h>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring>

#define STEPS 100000000
#define CACHE_LINE_SIZE 64u


double IntegrateMutex(unary_function F, double a, double b)
{
    using namespace std;
    vector<thread> Threads;
    mutex Mutex;
    unsigned int T = GetThreadCount();
    double dx = (b-a)/STEPS;
    double Result = 0;

    auto ThreadProcedure = [=, &Result, &Mutex](auto t)
    {
        double Accum = 0;
        for(unsigned int i = t; i < STEPS; i+=T)
        {
            Accum += F(dx*i + a);
        }
        Mutex.lock();
        Result += Accum;
        Mutex.unlock();
    };

    for(unsigned int t = 1; t < T; t++)
    {
        Threads.emplace_back(ThreadProcedure, t);
    }

    ThreadProcedure(0);
    for(auto &Thread : Threads)
        Thread.join();

    Result *= dx;
    return Result;
}

double IntegrateAtomic(unary_function F, double a, double b)
{
    std::vector<std::thread> Threads;
    std::atomic<double> Result;
    unsigned int T = GetThreadCount();
    double dx = (b-a)/STEPS;

    auto ThreadProcedure = [=, &Result](auto t)
    {
        double Accum = 0;
        for(unsigned int i = t; i < STEPS; i+=T)
        {
            Accum += F(dx*i + a);
        }
        Result += Accum;
    };

    for(unsigned int t = 1; t < T; t++)
    {
        Threads.emplace_back(ThreadProcedure, t);
    }

    ThreadProcedure(0);
    for(auto &Thread : Threads)
        Thread.join();

    Result = Result * dx;
    return Result;
}


double IntegratePS(unary_function F, double a, double b)
{
    double Result = 0;
    double dx = (b-a)/STEPS;
    unsigned int T = GetThreadCount();
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
