#pragma once

#include "HAL/Platform.h"
#include "UObject/UObjectArray.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UObject;

struct FWeakObjectPtr
{
public:
    FORCEINLINE FWeakObjectPtr()
    {
        Reset();
    }

    FORCEINLINE FWeakObjectPtr(const UObject* Object)
    {
        (*this) = Object;
    }

    FORCEINLINE void Reset()
    {
        ObjectIndex = -1;
        ObjectSerialNumber = 0;
    }

    void operator=(const UObject* Object);

    FORCEINLINE bool operator==(const FWeakObjectPtr& Other) const
    {
        return (ObjectIndex == Other.ObjectIndex && ObjectSerialNumber == Other.ObjectSerialNumber) || (!IsValid() && !Other.IsValid());
    }

    FORCEINLINE bool HasSameIndexAndSerialNumber(const FWeakObjectPtr& Other) const
    {
        return ObjectIndex == Other.ObjectIndex && ObjectSerialNumber == Other.ObjectSerialNumber;
    }

    UObject* Get() const;

    bool IsValid() const;

    friend uint32 GetTypeHash(const FWeakObjectPtr& WeakObjectPtr)
    {
        return uint32(WeakObjectPtr.ObjectIndex ^ WeakObjectPtr.ObjectSerialNumber);
    }

private:
    FORCEINLINE bool SerialNumbersMatch(FUObjectItem* ObjectItem) const
    {
        const int32 ActualSerialNumber = ObjectItem->GetSerialNumber();

        return ActualSerialNumber == ObjectSerialNumber;
    }

    FORCEINLINE bool Internal_IsValid(bool bEvenIfPendingKill, bool bThreadsafeTest) const
    {
        if (ObjectSerialNumber == 0)
        {
            return false;
        }

        if (ObjectIndex < 0)
        {
            return false;
        }

        FUObjectItem* ObjectItem = GUObjectArray.IndexToObject(ObjectIndex);

        if (!ObjectItem)
        {
            return false;
        }

        if (!SerialNumbersMatch(ObjectItem))
        {
            return false;
        }

        if (bThreadsafeTest)
        {
            return true;
        }

        return GUObjectArray.IsValid(ObjectItem, bEvenIfPendingKill);
    }

    FORCEINLINE UObject* Internal_Get(bool bEvenIfPendingKill) const
    {
        UObject* Result = nullptr;

        if (Internal_IsValid(true, true))
        {
            FUObjectItem* ObjectItem = GUObjectArray.IndexToValidObject(ObjectIndex, bEvenIfPendingKill);

            if (ObjectItem)
            {
                Result = (UObject*)ObjectItem->Object;
            }
        }

        return Result;
    }

    int32 ObjectIndex;
    int32 ObjectSerialNumber;
};
