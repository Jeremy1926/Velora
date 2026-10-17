#pragma once

#include "HAL/Platform.h"
#include "Containers/ContainerAllocationPolicies.h"

template<typename Allocator = FDefaultBitArrayAllocator>
class TBitArray
{
public:
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

private:
    FORCEINLINE uint32* GetData() const
    {
        return AllocatorInstance.GetAllocation();
    }

    typename Allocator::template ForElementType<uint32> AllocatorInstance;
    int32 NumBits;
    int32 MaxBits;
};
