#pragma once

#include "HAL/Platform.h"
#include "UObject/ObjectMacros.h"

class UObjectBase;

struct FUObjectItem
{
    UObjectBase* Object;
    int32 Flags;
    int32 ClusterRootIndex;
    int32 SerialNumber;

    FORCEINLINE int32 GetSerialNumber() const
    {
        return SerialNumber;
    }

    FORCEINLINE void SetRootSet()
    {
        Flags |= int32(EInternalObjectFlags::RootSet);
    }

    FORCEINLINE bool IsPendingKill() const
    {
        return !!(Flags & int32(EInternalObjectFlags::PendingKill));
    }

    FORCEINLINE bool IsUnreachable() const
    {
        return !!(Flags & int32(EInternalObjectFlags::Unreachable));
    }
};

class FFixedUObjectArray
{
public:
    FUObjectItem* Objects;
    int32 MaxElements;
    int32 NumElements;

    FORCEINLINE int32 Num() const
    {
        return NumElements;
    }

    FORCEINLINE FUObjectItem const& operator[](int32 Index) const
    {
        return Objects[Index];
    }
};

class FUObjectArray
{
public:
    FORCEINLINE int32 GetObjectArrayNum() const
    {
        return ObjObjects.Num();
    }

    FORCEINLINE FUObjectItem* IndexToObject(int32 Index)
    {
        if (Index < ObjObjects.Num())
            return const_cast<FUObjectItem*>(&ObjObjects[Index]);

        return nullptr;
    }

    FORCEINLINE bool IsValid(FUObjectItem* ObjectItem, bool bEvenIfPendingKill)
    {
        if (ObjectItem)
            return bEvenIfPendingKill ? !ObjectItem->IsUnreachable() : !(ObjectItem->IsUnreachable() || ObjectItem->IsPendingKill());

        return false;
    }

    FORCEINLINE FUObjectItem* IndexToValidObject(int32 Index, bool bEvenIfPendingKill)
    {
        FUObjectItem* ObjectItem = IndexToObject(Index);

        return IsValid(ObjectItem, bEvenIfPendingKill) ? ObjectItem : nullptr;
    }

    typedef FFixedUObjectArray TUObjectArray;

    int32 ObjFirstGCIndex;
    int32 ObjLastNonGCIndex;
    int32 MaxObjectsNotConsideredByGC;
    bool OpenForDisregardForGC;
    TUObjectArray ObjObjects;
};

extern FUObjectArray& GUObjectArray;
