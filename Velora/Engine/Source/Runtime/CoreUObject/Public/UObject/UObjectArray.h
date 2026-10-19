#pragma once

#include "HAL/Platform.h"
#include "Misc/EngineVersion.h"
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

class FChunkedFixedUObjectArray
{
public:
    FUObjectItem** Objects;
    FUObjectItem* PreAllocatedObjects;
    int32 MaxElements;
    int32 NumElements;
    int32 MaxChunks;
    int32 NumChunks;

    FORCEINLINE int32 Num() const
    {
        return NumElements;
    }

    FORCEINLINE FUObjectItem const& operator[](int32 Index) const
    {
        return Objects[Index / (MaxElements / MaxChunks)][Index % (MaxElements / MaxChunks)];
    }
};

class FUObjectArray
{
public:
    FORCEINLINE int32 GetObjectArrayNum() const
    {
        auto& Version = FEngineVersion::Current();

        return Version.GetMajor() > 4 || Version.GetMinor() >= 21 ? ((const FChunkedFixedUObjectArray&)ObjObjects).Num() : ObjObjects.Num();
    }

    FORCEINLINE FUObjectItem* IndexToObject(int32 Index)
    {
        auto& Version = FEngineVersion::Current();

        if (Index < GetObjectArrayNum())
            return const_cast<FUObjectItem*>(Version.GetMajor() > 4 || Version.GetMinor() >= 21 ? &((const FChunkedFixedUObjectArray&)ObjObjects)[Index] : &ObjObjects[Index]);

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
