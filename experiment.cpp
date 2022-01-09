struct integration_experiment_result
{
    double Result;
    double Time; // in seconds
};

typedef double (*integrate_function) (unary_function, double, double);
integration_experiment_result RunIntegrationExperiment(integrate_function I)
{
    double t0 = omp_get_wtime();
    double res = I(Quadratic, -1, 1);
    double t1 = omp_get_wtime();

    integration_experiment_result Result;
    Result.Result = res;
    Result.Time = t1 - t0;

    return Result;
}

void ShowIntegrationExperimentResult(integrate_function I, const char* Name)
{
    printf("%s\n", Name);
    SetThreadCount(1);

    printf("%10s, %10s %10ss %14s\n", "Threads", "Result", "TimeMS", "Acceleration");
    integration_experiment_result Experiment;
    Experiment = RunIntegrationExperiment(I);
    printf("%10d, %10g %10gs %14g\n", 1, Experiment.Result, Experiment.Time, 1.0f);

    double SingleThreadedTime = Experiment.Time;
    for(unsigned T = 2; T <=omp_get_num_procs(); T++)
    {
        SetThreadCount(T);
        Experiment = RunIntegrationExperiment(I);
        printf("%10d, %10g %10gs %14g\n", T, Experiment.Result, Experiment.Time, SingleThreadedTime/Experiment.Time);
    }
    printf("\n");
}

#define SHOW_INTEGRATION_EXPERIMENT(fun) ShowIntegrationExperimentResult((fun), (#fun))

typedef double (*randomize_function)(u64, u32*, u32, u32, u32);

struct randomization_experiment_result
{
    double Average;
    double ExpectedAverage;
    double DifferenceAverage;
    double Time;
};

// NOTE: As far as I understand, it will be the best for perfomance
// if array is cache line aligned
randomization_experiment_result RunRandomizationExperiment(randomize_function Randomize, u32* Array, u32 ArrayLength)
{
    u64 Seed = 93821;
    u32 Min = 100;
    u32 Max = 1500;

    double ExpectedAverage = 0.5f*(double)(Min + Max);

    double T0 = omp_get_wtime();
    double Average = Randomize(Seed, Array, ArrayLength, Min, Max);
    double T1 = omp_get_wtime();

    randomization_experiment_result Result;
    Result.Average = Average;
    Result.ExpectedAverage = ExpectedAverage;
    Result.DifferenceAverage = Result.ExpectedAverage - Result.Average;
    Result.Time = T1 - T0;

    return Result;
}

void ShowRandomizationExperimentResult(randomize_function Randomize, u32* Array, u32 ArrayLength, const char* Name)
{
    printf("%s (%d elements)\n", Name, ArrayLength);
    SetThreadCount(1);

    u32 Width = 4;
    printf("%-*s %-*s %-*s %-*s %-*s %-*s\n",
            (int)(strlen("Threads") + Width),           "Threads",
            (int)(strlen("Expected Average") + Width),  "Expected Average",
            (int)(strlen("Average") + Width),           "Average",
            (int)(strlen("DifferenceAverage") + Width), "DifferenceAverage",
            (int)(strlen("Time(s)") + Width),           "Time(s)",
            (int)(strlen("Acceleration") + Width),      "Acceleration");

    randomization_experiment_result Experiment;
    Experiment = RunRandomizationExperiment(Randomize, Array, ArrayLength);
    printf("%-*u %-*f %-*f %-*f %-*f %-*f\n",
            (int)(strlen("Threads") + Width),           1,
            (int)(strlen("Expected Average") + Width),  Experiment.ExpectedAverage,
            (int)(strlen("Average") + Width),           Experiment.Average,
            (int)(strlen("DifferenceAverage") + Width), Experiment.DifferenceAverage,
            (int)(strlen("Time(s)") + Width),           Experiment.Time,
            (int)(strlen("Acceleration") + Width),      1.0);

    double SingleThreadedTime = Experiment.Time;
    for(unsigned T = 2; T <=omp_get_num_procs(); T++)
    {
        SetThreadCount(T);
        Experiment = RunRandomizationExperiment(Randomize, Array, ArrayLength);
        printf("%-*u %-*f %-*f %-*f %-*f %-*f\n",
                (int)(strlen("Threads") + Width),           T,
                (int)(strlen("Expected Average") + Width),  Experiment.ExpectedAverage,
                (int)(strlen("Average") + Width),           Experiment.Average,
                (int)(strlen("DifferenceAverage") + Width), Experiment.DifferenceAverage,
                (int)(strlen("Time(s)") + Width),           Experiment.Time,
                (int)(strlen("Acceleration") + Width),      SingleThreadedTime/Experiment.Time);
    }
    printf("\n");
}

#define SHOW_RANDOMIZATION_EXPERIMENT(Fun, Array, Length) ShowRandomizationExperimentResult((Fun), (Array), (Length), (#Fun))
