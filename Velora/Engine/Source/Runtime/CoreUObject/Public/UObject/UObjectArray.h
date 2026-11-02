#pragma once

#include <Anchor/Anchor.h>
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

    UObjectBase* GetObject() const;

    int32 GetSerialNumber() const;

    volatile long* GetSerialNumberAddress();

    int32& GetFlags() const;

    FORCEINLINE void SetRootSet()
    {
        GetFlags() |= int32(EInternalObjectFlags::RootSet);
    }

    FORCEINLINE bool IsPendingKill() const
    {
        return !!(GetFlags() & int32(EInternalObjectFlags::PendingKill));
    }

    FORCEINLINE bool IsUnreachable() const
    {
        return !!(GetFlags() & int32(EInternalObjectFlags::Unreachable));
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
    struct FEncryptedLayout
    {
        Anchor::FCipher NumElements;
        Anchor::FCipher Objects;
        Anchor::FCipher SerialNumber;
        Anchor::FCipher Flags;
        Anchor::FCipher Object;
        int32 Stride;
    };

    static const FEncryptedLayout* GetEncryptedLayout();

    FORCEINLINE int32 GetObjectArrayNum() const
    {
        if (auto Layout = GetEncryptedLayout())
            return Layout->NumElements.Load<int32>(nullptr);

        auto& Version = FEngineVersion::Current();

        return Version.GetMajor() > 4 || Version.GetMinor() >= 21 ? ((const FChunkedFixedUObjectArray&)ObjObjects).Num() : ObjObjects.Num();
    }

    FORCEINLINE FUObjectItem* IndexToObject(int32 Index)
    {
        if (auto Layout = GetEncryptedLayout())
            return Index >= 0 && Index < GetObjectArrayNum() ? (FUObjectItem*)(Layout->Objects.Load<uint8**>(nullptr)[Index / (64 * 1024)] + Index % (64 * 1024) * Layout->Stride) : nullptr;

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

FORCEINLINE UObjectBase* FUObjectItem::GetObject() const
{
    auto Layout = FUObjectArray::GetEncryptedLayout();

    return Layout ? Layout->Object.Load<UObjectBase*>(this) : Object;
}

FORCEINLINE int32 FUObjectItem::GetSerialNumber() const
{
    auto Layout = FUObjectArray::GetEncryptedLayout();

    return Layout ? Layout->SerialNumber.Load<int32>(this) : SerialNumber;
}

FORCEINLINE volatile long* FUObjectItem::GetSerialNumberAddress()
{
    auto Layout = FUObjectArray::GetEncryptedLayout();

    return (volatile long*)(Layout ? (uint8*)this + Layout->SerialNumber.Offset : (uint8*)&SerialNumber);
}

FORCEINLINE int32& FUObjectItem::GetFlags() const
{
    auto Layout = FUObjectArray::GetEncryptedLayout();

    return *(int32*)(Layout ? (uint8*)this + Layout->Flags.Offset : (uint8*)&Flags);
}
