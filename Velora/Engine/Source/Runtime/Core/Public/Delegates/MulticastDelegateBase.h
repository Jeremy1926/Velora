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
public:
    bool IsBoundToObject(const void* InUserObject) const
    {
        for (const auto& DelegateBaseRef : InvocationList)
        {
            auto DelegateInstance = DelegateBaseRef.GetDelegateInstanceProtected();

            if (DelegateInstance && IDelegateInstance::HasSameObject(DelegateInstance, InUserObject))
                return true;
        }

        return false;
    }

    int32 RemoveAll(const void* InUserObject)
    {
        int32 Result = 0;

        if (InvocationListLockCount > 0)
        {
            for (auto& DelegateBaseRef : InvocationList)
            {
                auto DelegateInstance = DelegateBaseRef.GetDelegateInstanceProtected();

                if (DelegateInstance && IDelegateInstance::HasSameObject(DelegateInstance, InUserObject))
                {
                    DelegateBaseRef.Unbind();
                    ++Result;
                }
            }

            if (Result > 0)
                CompactionThreshold = 0;
        }
        else
        {
            for (int32 Index = 0; Index < InvocationList.Num();)
            {
                auto DelegateInstance = InvocationList[Index].GetDelegateInstanceProtected();

                if (!DelegateInstance || IDelegateInstance::HasSameObject(DelegateInstance, InUserObject) || IDelegateInstance::IsCompactable(DelegateInstance))
                {
                    InvocationList.RemoveAtSwap(Index, 1, false);
                    ++Result;
                }
                else
                {
                    ++Index;
                }
            }

            CompactionThreshold = FMath::Max(2, 2 * InvocationList.Num());
            InvocationList.Shrink();
        }

        return Result;
    }

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

            Result = IDelegateInstance::GetHandle(DelegateInstance);
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

            if (!DelegateInstance || IDelegateInstance::IsCompactable(DelegateInstance))
                InvocationList.RemoveAtSwap(Index, 1, false);
            else
                Index++;
        }

        CompactionThreshold = FMath::Max(2, 2 * InvocationList.Num());
    }

    inline const TInvocationList& GetInvocationList() const
    {
        return InvocationList;
    }

    inline void LockInvocationList() const
    {
        ++InvocationListLockCount;
    }

    inline void UnlockInvocationList() const
    {
        --InvocationListLockCount;
    }

    static inline void* GetDelegateInstanceProtectedHelper(const FDelegateBase& Base)
    {
        return Base.GetDelegateInstanceProtected();
    }

private:
    TInvocationList InvocationList;
    int32 CompactionThreshold;
    mutable int32 InvocationListLockCount;
};
