// TODO: header guards

static unsigned num_treads = std::thread::hardware_concurrency();
unsigned get_num_threads()
{
    return num_treads;
}

//TODO: omp set threads
void set_num_threads(unsigned t)
{
    num_treads = t;
    omp_set_num_threads(t);
}
