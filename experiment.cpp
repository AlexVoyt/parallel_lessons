struct experiment_result
{
    double Result;
    double TimeMS;
};

typedef double (*integrate_function) (unary_function, double, double);
experiment_result RunExperiment(integrate_function I)
{
    double t0 = omp_get_wtime();
    double res = I(Quadratic, -1, 1);
    double t1 = omp_get_wtime();

    experiment_result Result;
    Result.Result = res;
    Result.TimeMS = t1 - t0;

    return Result;
}

void ShowExperimentResult(integrate_function I, const char* Name)
{
    printf("%s\n", Name);
    SetThreadCount(1);

    printf("%10s, %10s %10sms %14s\n", "Threads", "Result", "TimeMS", "Acceleration");
    experiment_result Experiment;
    Experiment = RunExperiment(I);
    printf("%10d, %10g %10gms %14g\n", 1, Experiment.Result, Experiment.TimeMS, 1.0f);
    double Time = Experiment.TimeMS; // TODO: rename

    for(unsigned T = 2; T <=omp_get_num_procs(); T++)
    {
        SetThreadCount(T);
        Experiment = RunExperiment(I);
        printf("%10d, %10g %10gms %14g\n", T, Experiment.Result, Experiment.TimeMS, Time/Experiment.TimeMS);
    }
    printf("\n");
}

#define SHOW_EXPERIMENT(fun) ShowExperimentResult((fun), (#fun))
