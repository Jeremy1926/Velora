#pragma once

#include "HAL/Platform.h"
#include <xmmintrin.h>

struct FMath
{
    template<class T>
    static FORCEINLINE T Max(const T A, const T B)
    {
        return (A >= B) ? A : B;
    }

    template<class T>
    static FORCEINLINE T Min(const T A, const T B)
    {
        return (A <= B) ? A : B;
    }

    template<class T>
    static FORCEINLINE T Clamp(const T X, const T Min, const T Max)
    {
        return X < Min ? Min : X < Max ? X : Max;
    }

    template<class T, class U>
    static FORCEINLINE T Lerp(const T& A, const T& B, const U& Alpha)
    {
        return (T)(A + Alpha * (B - A));
    }

    template<class T>
    static FORCEINLINE T DivideAndRoundUp(T Dividend, T Divisor)
    {
        return (Dividend + Divisor - 1) / Divisor;
    }

    static FORCEINLINE uint32 CountLeadingZeros(uint32 Value)
    {
        return std::countl_zero(Value);
    }

    static FORCEINLINE uint32 CeilLogTwo(uint32 Arg)
    {
        int32 Bitmask = ((int32)(CountLeadingZeros(Arg) << 26)) >> 31;

        return (32 - CountLeadingZeros(Arg - 1)) & (~Bitmask);
    }

    static FORCEINLINE uint32 RoundUpToPowerOfTwo(uint32 Arg)
    {
        return 1 << CeilLogTwo(Arg);
    }

    static FORCEINLINE int32 TruncToInt(float F)
    {
        return _mm_cvtt_ss2si(_mm_set_ss(F));
    }

    static FORCEINLINE float TruncToFloat(float F)
    {
        return (float)TruncToInt(F);
    }

    static FORCEINLINE int32 RoundToInt(float F)
    {
        return _mm_cvt_ss2si(_mm_set_ss(F + F + 0.5f)) >> 1;
    }

    static FORCEINLINE float Fractional(float Value)
    {
        return Value - TruncToFloat(Value);
    }

    static FORCEINLINE int32 Rand()
    {
        return rand();
    }

    static FORCEINLINE float FRand()
    {
        return Rand() / (float)RAND_MAX;
    }

    static float SRand()
    {
        static int32 GSRandSeed;

        GSRandSeed = (GSRandSeed * 196314165) + 907633515;

        union { float f; int32 i; } Result;
        union { float f; int32 i; } Temp;
        Temp.f = 1.0f;
        Result.i = (Temp.i & 0xff800000) | (GSRandSeed & 0x007fffff);

        return Fractional(Result.f);
    }
};
