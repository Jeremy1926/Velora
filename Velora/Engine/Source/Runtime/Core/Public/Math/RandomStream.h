#pragma once

#include "HAL/Platform.h"
#include "Math/UnrealMathUtility.h"

struct FRandomStream
{
    FORCEINLINE float GetFraction() const
    {
        MutateSeed();

        union { float f; int32 i; } Result;
        union { float f; int32 i; } Temp;
        Temp.f = 1.0f;
        Result.i = (Temp.i & 0xff800000) | (Seed & 0x007fffff);

        return FMath::Fractional(Result.f);
    }

    FORCEINLINE int32 RandHelper(int32 A) const
    {
        return A > 0 ? FMath::TruncToInt(GetFraction() * float(A)) : 0;
    }

    FORCEINLINE int32 RandRange(int32 Min, int32 Max) const
    {
        const int32 Range = (Max - Min) + 1;

        return Min + RandHelper(Range);
    }

    FORCEINLINE void MutateSeed() const
    {
        Seed = (Seed * 196314165) + 907633515;
    }

    int32 InitialSeed;
    mutable int32 Seed;
};
