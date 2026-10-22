#pragma once

#include "HAL/Platform.h"
#include "Misc/CoreMiscDefines.h"
#include "UObject/WeakObjectPtr.h"

class UObject;

struct FObjectKey
{
public:
    FORCEINLINE FObjectKey()
        : ObjectIndex(INDEX_NONE)
        , ObjectSerialNumber(0)
    {
    }

    FORCEINLINE FObjectKey(const UObject* Object)
        : ObjectIndex(INDEX_NONE)
        , ObjectSerialNumber(0)
    {
        if (Object)
        {
            FWeakObjectPtr Weak(Object);
            ObjectIndex = Weak.ObjectIndex;
            ObjectSerialNumber = Weak.ObjectSerialNumber;
        }
    }

    FORCEINLINE bool operator==(const FObjectKey& Other) const
    {
        return ObjectIndex == Other.ObjectIndex && ObjectSerialNumber == Other.ObjectSerialNumber;
    }

    FORCEINLINE bool operator!=(const FObjectKey& Other) const
    {
        return ObjectIndex != Other.ObjectIndex || ObjectSerialNumber != Other.ObjectSerialNumber;
    }

private:
    int32 ObjectIndex;
    int32 ObjectSerialNumber;
};
