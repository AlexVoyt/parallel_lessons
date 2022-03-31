unsigned Fibonacci(unsigned n)
{
	if (n <= 2)
		return 1;
	return Fibonacci(n - 1) + Fibonacci(n - 2);
}

unsigned Fibonacci_omp(unsigned n)
{
	if (n <= 2)
		return 1;
	unsigned x1, x2;
	#pragma omp task shared(x1)
	{
		x1 = Fibonacci_omp(n - 1);
	}
	#pragma omp task shared(x2)
	{
		x2 = Fibonacci_omp(n - 2);
	}
	#pragma omp taskwait
	return x1 + x2;
}

#include <chrono>
#include <iostream>
#include "fib.h"

unsigned Fibonacci_queue(unsigned n)
{
    unsigned result;
    auto ctrl = std::make_shared<control_state_fib_blocking>(&result);
    auto task = std::make_unique<task_fib>(n, ctrl, &result);
    load_balancer::get().add_task(std::move(task));
    ctrl->wait();

    return result;
}

int main(int argc, char** argv)
{
    auto fib = [](auto fibonacci)
    {
        for (unsigned n = 0; n <= 30; ++n)
        {
            auto tm0 = std::chrono::steady_clock::now();
            unsigned res = fibonacci(n);
            auto dur = std::chrono::steady_clock::now() - tm0;
            std::cout << "n: " << n << ". Result: " << res << ". Time " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "\n";
        }
    };

    //fib(Fibonacci_omp);
    fib(Fibonacci_queue);
	return 0;
}

