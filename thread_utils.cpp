// TODO: header guards

static unsigned _ThreadCount = std::thread::hardware_concurrency();
unsigned GetThreadCount()
{
    return _ThreadCount;
}

void SetThreadCount(unsigned t)
{
    _ThreadCount = t;
    omp_set_num_threads(t);
}
