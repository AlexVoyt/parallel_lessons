double RandomizeArraySingleThreaded(u64 Seed, u32* Array, u32 ArrayLength, u32 Min, u32 Max)
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

// TODO: fast exponentiation
u64 Power(u64 Base, u64 Exponent)
{
    assert(Exponent >= 0);
    u64 Result = 1;
    for(u32 I = 0; I < Exponent; I++)
    {
        Result *= Base;
    }

    return Result;
}

double RandomizeArrayFalseSharing(u64 Seed, u32* Array, u32 ArrayLength, u32 Min, u32 Max)
{
    assert(Min <= Max);
    u32 ThreadCount = GetThreadCount();
    u64* GenerationStateTable = (u64* )Allocate(sizeof(*GenerationStateTable)*ThreadCount);

    struct aligned_sum
    {
        alignas(GetCacheLineSize()) u64 Value;
    };

    aligned_sum* SumTable = (aligned_sum* )AllocateAlign(ThreadCount*sizeof(*SumTable), GetCacheLineSize());
    for(u32 I = 0; I < ThreadCount; I++)
    {
        SumTable[I].Value = 0;
    }

    u64 A = 6364136223846793005;
    u64 B = 1;

    u64 PrevValue = Seed;
    u64 Sum = 0;
    for(u32 I = 0; I < ThreadCount; I++)
    {
        u64 NextValue = A*PrevValue + B;
        GenerationStateTable[I] = NextValue;
        Array[I] = (NextValue % (Max - Min + 1)) + Min;
        PrevValue = NextValue;
        Sum += Array[I];
    }

    u64 AT = Power(A, ThreadCount);
    u64 BT = 0;
    for(u32 I = 0; I < ThreadCount; I++)
    {
        BT += Power(A, I);
    }
    BT *= B;

    auto ThreadProcedure = [=](unsigned ThreadIndex, u64 GenerationState)
    {
        u64 Sum = 0;
        u64 PrevValue = GenerationState;
        for(u32 I = ThreadIndex + ThreadCount; I < ArrayLength; I += ThreadCount)
        {
            u64 NextValue = AT*PrevValue + BT;
            Array[I] = (NextValue % (Max - Min + 1)) + Min;
            PrevValue = NextValue;
            Sum += Array[I];
        }

        SumTable[ThreadIndex].Value += Sum;
    };

    std::vector<std::thread> Threads;
    for(unsigned ThreadIndex = 1; ThreadIndex < ThreadCount; ThreadIndex++)
        Threads.emplace_back(ThreadProcedure, ThreadIndex, GenerationStateTable[ThreadIndex]);
    ThreadProcedure(0, GenerationStateTable[0]);
    for(auto& Thread : Threads)
        Thread.join();

    for(u32 I = 0; I < ThreadCount; I++)
    {
        Sum += SumTable[I].Value;
    }

    Free(GenerationStateTable);
    FreeAlign(SumTable);
    return (double)Sum/(double)ArrayLength;
}

double RandomizeArray(u64 Seed, u32* Array, u32 ArrayLength, u32 Min, u32 Max)
{
    assert(Min <= Max);
    u32 ThreadCount = GetThreadCount();

    u32 CacheLineSize = GetCacheLineSize();
    u32 ElementSize = sizeof(*Array);
    u32 ElementsInBlock = CacheLineSize/ElementSize;

    assert(ElementSize == 4);
    assert(ElementsInBlock == 16);

    u64 A = 6364136223846793005;
    u64 B = 1;

    // TODO: can optimize this using fast power and remembering previous A multiples
    // that we calculated for StrideB
    u32 BlockStride = ElementsInBlock;
    u64 BlockStrideA = Power(A, BlockStride);
    u64 BlockStrideB = 0;
    for(u32 I = 0; I < BlockStride; I++)
    {
        BlockStrideB += Power(A, I);
    }
    BlockStrideB *= B;

    u32 ThreadStride = ThreadCount*ElementsInBlock;
    u64 ThreadStrideA = Power(A, ThreadStride);
    u64 ThreadStrideB = 0;
    for(u32 I = 0; I < ThreadStride; I++)
    {
        ThreadStrideB += Power(A, I);
    }
    ThreadStrideB *= B;

    auto ToInterval = [](u64 Value, u32 Min, u32 Max)
    {
        return (Value % (Max - Min + 1)) + Min;
    };

    // Init first elements in the first ThreadCount blocks and save LCG states to table
    u32 InitialElementIndex = 0;
    u64 Value = A*Seed + B;

    u64* GenerationStateTable = (u64* )Allocate(sizeof(*GenerationStateTable)*ThreadCount);
    u64 Sum = 0;
    for(u32 I = 0; I < ThreadCount; I++)
    {
        if(InitialElementIndex < ArrayLength)
        {
            Array[InitialElementIndex] = ToInterval(Value, Min, Max);
            Sum += Array[InitialElementIndex];
            GenerationStateTable[I] = Value;
            Value = BlockStrideA*Value + BlockStrideB;
            InitialElementIndex += BlockStride;
        }
    }

    struct aligned_sum
    {
        alignas(GetCacheLineSize()) u64 Value;
    };

    aligned_sum* SumTable = (aligned_sum* )AllocateAlign(ThreadCount*sizeof(*SumTable), CacheLineSize);
    for(u32 I = 0; I < ThreadCount; I++)
    {
        SumTable[I].Value = 0;
    }

    auto ThreadProcedure = [=](unsigned ThreadIndex, u64 GenerationState)
    {
        u64 Sum = 0;
        bool Generating = true;
        u32 BlockIndex = BlockStride*ThreadIndex;
        u64 GenerationValue = GenerationState;
        while(Generating)
        {
            for(u32 ElementIndex = 1; ElementIndex < ElementsInBlock; ElementIndex++)
            {
                if(BlockIndex + ElementIndex < ArrayLength)
                {
                    GenerationValue = A*GenerationValue + B;
                    Array[BlockIndex + ElementIndex] = ToInterval(GenerationValue, Min, Max);
                    Sum += Array[BlockIndex + ElementIndex];
                }
                else
                {
                    Generating = false;
                    break;
                }
            }

            GenerationValue = ThreadStrideA*GenerationState + ThreadStrideB;
            BlockIndex = BlockIndex + ThreadStride;
            if(BlockIndex < ArrayLength)
            {
                Array[BlockIndex] = ToInterval(GenerationValue, Min, Max);
                Sum += Array[BlockIndex];
            }
            else
            {
                Generating = false;
            }
            GenerationState = GenerationValue;

            if(!Generating)
            {
                break;
            }
        }

        SumTable[ThreadIndex].Value += Sum;
    };

    std::vector<std::thread> Threads;
    for(unsigned ThreadIndex = 1; ThreadIndex < ThreadCount; ThreadIndex++)
        Threads.emplace_back(ThreadProcedure, ThreadIndex, GenerationStateTable[ThreadIndex]);
    ThreadProcedure(0, GenerationStateTable[0]);
    for(auto& Thread : Threads)
        Thread.join();

    for(u32 I = 0; I < ThreadCount; I++)
    {
        Sum += SumTable[I].Value;
    }

    Free(GenerationStateTable);
    FreeAlign(SumTable);

    return (double)Sum/(double)ArrayLength;
}
