std::size_t ceil_div(std::size_t x, std::size_t y)
{
    return (x + y - 1) / y;
}

template <class ElementType, class BinaryFn>
ElementType reduce_vector(const ElementType* V, std::size_t n, BinaryFn f, ElementType zero)
{
    unsigned T = get_num_threads();
    struct reduction_partial_result_t
    {
        alignas(std::hardware_destructive_interference_size) ElementType value;
    };
    static auto reduction_partial_results =
        std::vector<reduction_partial_result_t>(std::thread::hardware_concurrency(), reduction_partial_result_t{zero});

    constexpr std::size_t k = 2;
    auto thread_proc = [=](unsigned t)
    {
        auto K = ceil_div(n, k);
        std::size_t Mt = K / T;
        std::size_t it1;

        if(t < (K % T))
        {
            it1 = ++Mt * t;
        }
        else
        {
            it1 = (K % T) * Mt + t;
        }
        it1 *= k;
        std::size_t mt = Mt * k;
        std::size_t it2 = it1 + mt;

        ElementType accum = zero;
        for(std::size_t i = it1; i < it2; i++)
            accum = f(accum, V[i]);

        reduction_partial_results[t].value = accum;
    };

#if 0
    auto thread_proc_2 = [=](unsigned t)
    {
        constexpr std::size_t k = 2;
        std::size_t s = 1;
        while((t % (s * k)) && (t + s < T))
        {
            reduction_partial_results[t].value = f(reduction_partial_results[t].value,
                                                   reduction_partial_results[t + s].value);
            s *= k;
            // ------------barrier------------ //
        }
    };
#endif

    auto thread_proc_2_ = [=](unsigned t, std::size_t s)
    {
        if(((t % (s * k)) == 0) && (t + s < T))
            reduction_partial_results[t].value = f(reduction_partial_results[t].value,
                                                   reduction_partial_results[t + s].value);
    };

    std::vector<std::thread> threads;
    for(unsigned t = 1; t < T; t++)
        threads.emplace_back(thread_proc, t);
    thread_proc(0);
    for(auto& thread : threads)
        thread.join();

    std::size_t s = 1;
    while(s < T)
    {
        for(unsigned t = 1; t < T; t++)
        {
            threads[t-1] = std::thread(thread_proc_2_, t, s);
        }
        thread_proc_2_(0, s);
        s *= k;

        for(auto& thread : threads)
            thread.join();
    }

#if 0

    for(unsigned t = 1; t < T; t++)
        threads[t-1] = std::thread(thread_proc_2, t);
    thread_proc_2(0);
    for(auto& thread : threads)
        thread.join();
#endif

    return reduction_partial_results[0].value;
}

#include <type_traits>

template <class ElementType, class UnaryFn, class BinaryFn>
#if 0
requires {
    std::is_invocable_r_v<UnaryFn, ElementType, ElementType> &&
    std::is_invocable_r_v<BinaryFn, ElementType, ElementType, ElementType>
}
#endif
ElementType reduce_range(ElementType a, ElementType b, std::size_t n, UnaryFn get, BinaryFn reduce_2, ElementType zero)
{
    unsigned T = get_num_threads();
    struct reduction_partial_result_t
    {
        alignas(std::hardware_destructive_interference_size) ElementType value;
    };
    static auto reduction_partial_results =
        std::vector<reduction_partial_result_t>(std::thread::hardware_concurrency(), reduction_partial_result_t{zero});

    constexpr std::size_t k = 2;
    auto thread_proc = [=](unsigned t)
    {
        auto K = ceil_div(n, k);
        double dx = (b - a) / n;
        std::size_t Mt = K / T;
        std::size_t it1;

        if(t < (K % T))
        {
            it1 = ++Mt * t;
        }
        else
        {
            it1 = (K % T) * Mt + t;
        }
        it1 *= k;
        std::size_t mt = Mt * k;
        std::size_t it2 = it1 + mt;

        ElementType accum = zero;
        for(std::size_t i = it1; i < it2; i++)
            accum = reduce_2(accum, get(a + i*dx));

        reduction_partial_results[t].value = accum;
    };

#if 0
    auto thread_proc_2 = [=](unsigned t)
    {
        constexpr std::size_t k = 2;
        std::size_t s = 1;
        while((t % (s * k)) && (t + s < T))
        {
            reduction_partial_results[t].value = f(reduction_partial_results[t].value,
                                                   reduction_partial_results[t + s].value);
            s *= k;
            // ------------barrier------------ //
        }
    };
#endif

    auto thread_proc_2_ = [=](unsigned t, std::size_t s)
    {
        if(((t % (s * k)) == 0) && (t + s < T))
            reduction_partial_results[t].value = reduce_2(reduction_partial_results[t].value,
                                                          reduction_partial_results[t + s].value);
    };

    std::vector<std::thread> threads;
    for(unsigned t = 1; t < T; t++)
        threads.emplace_back(thread_proc, t);
    thread_proc(0);
    for(auto& thread : threads)
        thread.join();

    std::size_t s = 1;
    while(s < T)
    {
        for(unsigned t = 1; t < T; t++)
        {
            threads[t-1] = std::thread(thread_proc_2_, t, s);
        }
        thread_proc_2_(0, s);
        s *= k;

        for(auto& thread : threads)
            thread.join();
    }

#if 0

    for(unsigned t = 1; t < T; t++)
        threads[t-1] = std::thread(thread_proc_2, t);
    thread_proc_2(0);
    for(auto& thread : threads)
        thread.join();
#endif

    return reduction_partial_results[0].value;
}

// TODO: pull this to own header and invoke in file where we have RunExperiment function
/*
double integrate_reduction(double a, double b, function F)
{
    return reduce_range(a, b, STEPS, F, [](auto x, auto y){return x + y}, 0);
}
*/

