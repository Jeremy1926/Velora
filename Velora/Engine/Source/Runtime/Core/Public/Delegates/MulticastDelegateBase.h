#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Delegates/DelegateBase.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/UnrealTemplate.h"

typedef TArray<FDelegateBase> TInvocationList;

template<typename ObjectPtrType>
class FMulticastDelegateBase
{
protected:
    inline FMulticastDelegateBase()
        : CompactionThreshold(2)
        , InvocationListLockCount(0)
    {
    }

    inline FDelegateHandle AddDelegateInstance(FDelegateBase&& NewDelegateBaseRef)
    {
        FDelegateHandle Result;

        if (auto DelegateInstance = NewDelegateBaseRef.GetDelegateInstanceProtected())
        {
            CompactInvocationList(true);

            Result = DelegateInstance->GetHandle();
            new (InvocationList) FDelegateBase(MoveTemp(NewDelegateBaseRef));
        }

        return Result;
    }

    void CompactInvocationList(bool CheckThreshold = false)
    {
        if (InvocationListLockCount > 0)
            return;

        if (CheckThreshold && CompactionThreshold > InvocationList.Num())
            return;

        for (int32 Index = 0; Index < InvocationList.Num();)
        {
            auto DelegateInstance = InvocationList[Index].GetDelegateInstanceProtected();

            if (!DelegateInstance || DelegateInstance->IsCompactable())
                InvocationList.RemoveAtSwap(Index, 1, false);
            else
                Index++;
        }

        CompactionThreshold = FMath::Max(2, 2 * InvocationList.Num());
    }

private:
    TInvocationList InvocationList;
    int32 CompactionThreshold;
    mutable int32 InvocationListLockCount;
};
