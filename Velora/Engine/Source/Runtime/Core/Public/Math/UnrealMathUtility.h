#pragma once

#include "HAL/Platform.h"
#include "Templates/UnrealTemplate.h"
#include <xmmintrin.h>

struct FVector;

namespace UE::LWC
{
    bool IsEnabled();
}

template<typename ValueType, typename DoubleStorageType>
struct TLWCGameLayout
{
    using FStorage = DoubleStorageType;

    static FORCEINLINE int32 Size()
    {
        return UE::LWC::IsEnabled() ? (int32)sizeof(DoubleStorageType) : (int32)sizeof(ValueType);
    }

    static FORCEINLINE ValueType Load(const void* Game)
    {
        if (UE::LWC::IsEnabled())
            return (ValueType)*(const DoubleStorageType*)Game;

        ValueType Value;
        memcpy(&Value, Game, sizeof(ValueType));

        return Value;
    }

    static FORCEINLINE void Store(void* Game, const ValueType& Value)
    {
        if (UE::LWC::IsEnabled())
            *(DoubleStorageType*)Game = DoubleStorageType(Value);
        else
            memcpy(Game, &Value, sizeof(ValueType));
    }
};

struct FMath
{
    static FVector VRand();

    static FVector VRandCone(FVector const& Dir, float ConeHalfAngleRad);

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
    static FORCEINLINE T Abs(const T A)
    {
        return (A >= (T)0) ? A : -A;
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

    static FORCEINLINE int32 FloorToInt(float F)
    {
        return _mm_cvt_ss2si(_mm_set_ss(F + F - 0.5f)) >> 1;
    }

    static FORCEINLINE int32 CeilToInt(float F)
    {
        return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (F + F))) >> 1);
    }

    static FORCEINLINE float Sqrt(float Value)
    {
        return sqrtf(Value);
    }

    static FORCEINLINE float InvSqrt(float F)
    {
        const __m128 fOneHalf = _mm_set_ss(0.5f);
        __m128 Y0, X0, X1, X2, FOver2;
        float temp;

        Y0 = _mm_set_ss(F);
        X0 = _mm_rsqrt_ss(Y0);
        FOver2 = _mm_mul_ss(Y0, fOneHalf);

        X1 = _mm_mul_ss(X0, X0);
        X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
        X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

        X2 = _mm_mul_ss(X1, X1);
        X2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X2));
        X2 = _mm_add_ss(X1, _mm_mul_ss(X1, X2));

        _mm_store_ss(&temp, X2);

        return temp;
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

    static FORCEINLINE float FRandRange(float InMin, float InMax)
    {
        return InMin + (InMax - InMin) * FRand();
    }

    static FORCEINLINE int32 RandHelper(int32 A)
    {
        return A > 0 ? Min(TruncToInt(FRand() * A), A - 1) : 0;
    }

    static FORCEINLINE int32 RandRange(int32 Min, int32 Max)
    {
        const int32 Range = (Max - Min) + 1;

        return Min + RandHelper(Range);
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
