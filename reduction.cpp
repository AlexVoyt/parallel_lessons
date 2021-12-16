std::size_t ceil_div(std::size_t x, std::size_t y)
{
    return (x + y - 1) / y;
}

template <class ElementType, class BinaryFn>
ElementType reduce_vector(const ElementType* V, std::size_t n, BinaryFn f, ElementType zero)
{
    unsigned T = GetThreadCount();
    struct reduction_partial_result_t
    {
        alignas(std::hardware_destructive_interference_size) ElementType value;
    };
    static auto reduction_partial_results =

        std::vector<reduction_partial_result_t>(std::thread::hardware_concurrency(), 
                                                reduction_partial_result_t{zero});
    constexpr std::size_t k = 2;
    std::barrier<> bar {T};

    auto thread_proc = [=, &bar](unsigned t)
    {
        auto K = ceil_div(n, k);
        std::size_t Mt = K / T;
        std::size_t it1 = K % T;

        if(t < it1)
        {
            it1 = ++Mt * t;
        }
        else
        {
            it1 = Mt * t + it1;
        }
        it1 *= k;
        std::size_t mt = Mt * k;
        std::size_t it2 = it1 + mt;

        ElementType accum = zero;
        for(std::size_t i = it1; i < it2; i++)
            accum = f(accum, V[i]);

        reduction_partial_results[t].value = accum;

#if 0
        std::size_t s = 1;
        while(s < T)
        {
            bar.arrive_and_wait();
            if((t % (s * k)) && (t + s < T))
            {
                reduction_partial_results[t].value = f(reduction_partial_results[t].value,
                                                       reduction_partial_results[t + s].value);
                s *= k;
            }
        }
#else
        for(std::size_t s = 1, s_next = 2; s < T; s = s_next, s_next += s_next) //TODO assume k = 2
        {
            bar.arrive_and_wait();
            if(((t % s_next) == 0) && (t + s < T))
                reduction_partial_results[t].value = f(reduction_partial_results[t].value,
                                                       reduction_partial_results[t + s].value);
        }
#endif
    };


    std::vector<std::thread> threads;
    for(unsigned t = 1; t < T; t++)
        threads.emplace_back(thread_proc, t);
    thread_proc(0);
    for(auto& thread : threads)
        thread.join();

    return reduction_partial_results[0].value;
}

#include <type_traits>

template <class ElementType, class UnaryFn, class BinaryFn>
// TODO: make this compile on all compilers ?
#if 0
requires {
    std::is_invocable_r_v<UnaryFn, ElementType, ElementType> &&
    std::is_invocable_r_v<BinaryFn, ElementType, ElementType, ElementType>
}
#endif
ElementType reduce_range(ElementType a, ElementType b, std::size_t n, UnaryFn get, BinaryFn reduce_2, ElementType zero)
{
    unsigned T = GetThreadCount();
    struct reduction_partial_result_t
    {
        alignas(std::hardware_destructive_interference_size) ElementType value;
    };
    static auto reduction_partial_results =
        std::vector<reduction_partial_result_t>(std::thread::hardware_concurrency(), reduction_partial_result_t{zero});

    std::barrier<> bar{T};
    constexpr std::size_t k = 2;
    auto thread_proc = [=, &bar](unsigned t)
    {
        auto K = ceil_div(n, k);
        double dx = (b - a) / n;
        std::size_t Mt = K / T;
        std::size_t it1 = K % T;

        if(t < it1)
        {
            it1 = ++Mt * t;
        }
        else
        {
            it1 = Mt * t + it1;
        }
        it1 *= k;
        std::size_t mt = Mt * k;
        std::size_t it2 = it1 + mt;

        ElementType accum = zero;
        for(std::size_t i = it1; i < it2; i++)
            accum = reduce_2(accum, get(a + i*dx));

        reduction_partial_results[t].value = accum;

        for(std::size_t s = 1, s_next = 2; s < T; s = s_next, s_next += s_next) //TODO assume k = 2
        {
            bar.arrive_and_wait();
            if(((t % s_next) == 0) && (t + s < T))
                reduction_partial_results[t].value = reduce_2(reduction_partial_results[t].value,
                                                              reduction_partial_results[t + s].value);
        }
    };

    std::vector<std::thread> threads;
    for(unsigned t = 1; t < T; t++)
        threads.emplace_back(thread_proc, t);
    thread_proc(0);
    for(auto& thread : threads)
        thread.join();
    return reduction_partial_results[0].value;
}

// TODO: pull this to own header and invoke in file where we have RunExperiment function

double IntegrateReduction(unary_function F, double a, double b)
{
    double dx = (b - a)/ STEPS;
    return reduce_range(a, b, STEPS, F, [](auto x, auto y){return x + y;}, 0.0)*dx;
}


