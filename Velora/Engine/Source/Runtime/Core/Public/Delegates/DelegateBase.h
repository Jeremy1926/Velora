#pragma once

#include "HAL/Platform.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Templates/TypeCompatibleBytes.h"

typedef TAlignedBytes<16, 16> AlignedInlineDelegateType;
typedef TInlineAllocator<2> DelegateAllocatorType;

class FDelegateBase
{
public:
    explicit FDelegateBase()
        : DelegateSize(0)
    {
    }

private:
    DelegateAllocatorType::ForElementType<AlignedInlineDelegateType> DelegateAllocator;
    int32 DelegateSize;
};
