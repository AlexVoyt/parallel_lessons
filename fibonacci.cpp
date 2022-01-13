unsigned Fibonacci(unsigned n)
{
    if (n <= 2)
    return 1;
    return Fibonacci(n - 1) + Fibonacci(n - 2);
}

#if 0 // doesn not work with visual studio
unsigned FibonacciOMP(unsigned n)
{
    if (n <= 2)
    return 1;

    unsigned x1, x2;
#pragma omp task
    {
        x1 = FibonacciOMP(n - 1);
    }

#pragma omp task
    {
        x2 = FibonacciOMP(n - 2);
    }

#pragma omp taskwait
    return x1 + x2;
}
#endif
