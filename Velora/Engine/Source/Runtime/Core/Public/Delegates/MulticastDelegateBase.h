#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Delegates/DelegateBase.h"

typedef TArray<FDelegateBase, TInlineAllocator<1>> TInvocationList;

template<typename ObjectPtrType>
class FMulticastDelegateBase
{
protected:
    inline FMulticastDelegateBase()
        : CompactionThreshold(2)
        , InvocationListLockCount(0)
    {
    }

private:
    TInvocationList InvocationList;
    int32 CompactionThreshold;
    mutable int32 InvocationListLockCount;
};
