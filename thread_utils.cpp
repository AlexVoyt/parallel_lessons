// TODO: header guards

static unsigned ThreadCount = std::thread::hardware_concurrency();
unsigned GetThreadCount()
{
    return ThreadCount;
}

void SetThreadCount(unsigned t)
{
    ThreadCount = t;
    omp_set_num_threads(t);
}
