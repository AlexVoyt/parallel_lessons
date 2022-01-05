// returns average
double RandomizeArray(u64 Seed, u32* Array, u32 ArrayLength, u32 Min, u32 Max)
{
    assert(Min <= Max);
    u64 A = 6364136223846793005;
    u64 B = 1;
    
    u32 ThreadCount = GetThreadCount();

    u64* LUTA = Allocate(ThreadCount*sizeof(*LUTA));
    u64* LUTB = Allocate(ThreadCount*sizeof(*LUTB));

    LUTA[0] = A;
    LUTB[0] = B;

    for(u32 I = 0; I < ThreadCount; I++)
    {
    }


    u64 PrevValue = Seed;
    for(u32 I = 0; I < ArrayLength; I++)
    {
        u64 NextValue = A*PrevValue + B;
        Array[I] = (NextValue % (Max - Min + 1)) + Min;
        PrevValue = NextValue;
    }

    free(LUTA);
    free(LUTB);

    return 0;
}

double RandomizeArraySingle(u64 Seed, u32* Array, u32 ArrayLength, u32 Min, u32 Max)
{
    assert(Min <= Max);
    u64 A = 6364136223846793005;
    u64 B = 1;
    
    u64 PrevValue = Seed;
    u64 Sum = 0;
    for(u32 I = 0; I < ArrayLength; I++)
    {
        u64 NextValue = A*PrevValue + B;
        Array[I] = (NextValue % (Max - Min + 1)) + Min;
        PrevValue = NextValue;
        Sum += Array[I];
    }

    return (double)Sum/(double)ArrayLength;
}

