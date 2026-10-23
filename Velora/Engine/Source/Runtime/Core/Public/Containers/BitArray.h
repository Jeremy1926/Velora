#pragma once

#include "HAL/Platform.h"
#include "HAL/UnrealMemory.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Math/UnrealMathUtility.h"

template<typename Allocator = FDefaultBitArrayAllocator>
class TBitArray
{
public:
    FORCEINLINE TBitArray()
        : NumBits(0)
        , MaxBits(0)
    {
    }

    FORCEINLINE ~TBitArray()
    {
        MaxBits = 0;
        NumBits = 0;
        Realloc(0);
    }

    FORCEINLINE int32 Num() const
    {
        return NumBits;
    }

    FORCEINLINE bool operator[](int32 Index) const
    {
        return (GetData()[Index / 32] & (1 << (Index & 31))) != 0;
    }

    FORCEINLINE void SetBit(int32 Index, bool Value)
    {
        if (Value)
            GetData()[Index / 32] |= 1 << (Index & 31);
        else
            GetData()[Index / 32] &= ~(1 << (Index & 31));
    }

    int32 Add(const bool Value)
    {
        const int32 Index = NumBits;
        const bool bReallocate = (NumBits + 1) > MaxBits;

        NumBits++;

        if (bReallocate)
        {
            const uint32 MaxDWORDs = AllocatorInstance.CalculateSlackGrow(FMath::DivideAndRoundUp(NumBits, 32), FMath::DivideAndRoundUp(MaxBits, 32), sizeof(uint32));
            MaxBits = MaxDWORDs * 32;
            Realloc(NumBits - 1);
        }

        SetBit(Index, Value);

        return Index;
    }

    void Reserve(int32 Number)
    {
        if (Number > MaxBits)
        {
            const uint32 MaxDWORDs = AllocatorInstance.CalculateSlackGrow(FMath::DivideAndRoundUp(Number, 32), FMath::DivideAndRoundUp(MaxBits, 32), sizeof(uint32));
            MaxBits = MaxDWORDs * 32;
            Realloc(NumBits);
        }
    }

    void Empty(int32 ExpectedNumBits = 0)
    {
        NumBits = 0;
        ExpectedNumBits = FMath::DivideAndRoundUp(ExpectedNumBits, 32) * 32;

        if (MaxBits != ExpectedNumBits)
        {
            MaxBits = ExpectedNumBits;
            Realloc(0);
        }
    }

    void Reset()
    {
        FMemory::Memset(GetData(), 0, FMath::DivideAndRoundUp(NumBits, 32) * sizeof(uint32));

        NumBits = 0;
    }

private:
    FORCEINLINE uint32* GetData() const
    {
        return AllocatorInstance.GetAllocation();
    }

    void Realloc(int32 PreviousNumBits)
    {
        const int32 PreviousNumDWORDs = FMath::DivideAndRoundUp(PreviousNumBits, 32);
        const int32 MaxDWORDs = FMath::DivideAndRoundUp(MaxBits, 32);

        AllocatorInstance.ResizeAllocation(PreviousNumDWORDs, MaxDWORDs, sizeof(uint32));

        if (MaxDWORDs)
        {
            FMemory::Memzero(GetData() + PreviousNumDWORDs, (MaxDWORDs - PreviousNumDWORDs) * sizeof(uint32));
        }
    }

    typename Allocator::template ForElementType<uint32> AllocatorInstance;
    int32 NumBits;
    int32 MaxBits;
};
