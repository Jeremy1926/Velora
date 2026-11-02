#pragma once

#include "UObject/WeakObjectPtr.h"
#include "UObject/SoftObjectPath.h"
#include "Misc/StringAssetReference.h"
#include "Misc/EngineVersion.h"
#include "Containers/Array.h"
#include "Templates/Casts.h"

template<class TObjectID>
struct TPersistentObjectPtr
{
public:
    FORCEINLINE TPersistentObjectPtr(const TPersistentObjectPtr& Other)
        : WeakPtr(Other.WeakPtr)
        , TagAtLastTest(Other.TagAtLastTest)
    {
        new(&GetObjectID()) TObjectID(Other.GetObjectID());
    }

    FORCEINLINE ~TPersistentObjectPtr()
    {
        GetObjectID().~TObjectID();
    }

    FORCEINLINE TPersistentObjectPtr& operator=(const TPersistentObjectPtr& Other)
    {
        WeakPtr = Other.WeakPtr;

        if (HasTagAtLastTest())
            TagAtLastTest = Other.TagAtLastTest;

        GetObjectID() = Other.GetObjectID();

        return *this;
    }

    FORCEINLINE void operator=(const UObject* Object)
    {
        if (Object)
        {
            GetObjectID() = TObjectID::GetOrCreateIDForObject(Object);
            WeakPtr = Object;

            if (HasTagAtLastTest())
                TagAtLastTest = TObjectID::GetCurrentTag();
        }
        else
        {
            WeakPtr.Reset();
            GetObjectID().Reset();

            if (HasTagAtLastTest())
                TagAtLastTest = 0;
        }
    }

    FORCEINLINE UObject* Get() const
    {
        return WeakPtr.Get();
    }

    FORCEINLINE bool IsValid() const
    {
        return !!Get();
    }

    FORCEINLINE bool IsPending() const
    {
        return !Get() && GetObjectID().IsValid();
    }

    FORCEINLINE bool IsNull() const
    {
        return !IsValid() && !GetObjectID().IsValid();
    }

    FORCEINLINE const TObjectID& GetUniqueID() const
    {
        return GetObjectID();
    }

    static FORCEINLINE int32 GetTypeSize()
    {
        return HasTagAtLastTest() ? (int32)sizeof(TPersistentObjectPtr) : (int32)(sizeof(FWeakObjectPtr) + sizeof(TObjectID));
    }

    UObject* LoadSynchronous() const
    {
        UObject* Asset = Get();

        if (IsPending())
            Asset = GetUniqueID().TryLoad();

        return Asset;
    }

private:
    static bool HasTagAtLastTest();

    FORCEINLINE TObjectID& GetObjectID() const
    {
        return *(TObjectID*)(HasTagAtLastTest() ? ObjectID : (const uint8*)&TagAtLastTest);
    }

    mutable FWeakObjectPtr WeakPtr;
    mutable int32 TagAtLastTest;
    alignas(TObjectID) uint8 ObjectID[sizeof(TObjectID)];
};

template<>
bool TPersistentObjectPtr<FSoftObjectPath>::HasTagAtLastTest();

template<>
FORCEINLINE bool TPersistentObjectPtr<FStringAssetReference>::HasTagAtLastTest()
{
    return true;
}

struct FAssetPtr : public TPersistentObjectPtr<FStringAssetReference>
{
};

struct FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath>
{
public:
    using TPersistentObjectPtr<FSoftObjectPath>::operator=;
};

template<class T>
class TAssetPtr
{
public:
    static int32 GetTypeSize()
    {
        return IsSoftObjectPtr() ? FSoftObjectPtr::GetTypeSize() : FAssetPtr::GetTypeSize();
    }

    FORCEINLINE bool IsNull() const
    {
        return IsSoftObjectPtr() ? ((const FSoftObjectPtr*)this)->IsNull() : ((const FAssetPtr*)this)->IsNull();
    }

    T* LoadSynchronous() const
    {
        return Cast<T>(IsSoftObjectPtr() ? ((const FSoftObjectPtr*)this)->LoadSynchronous() : ((const FAssetPtr*)this)->LoadSynchronous());
    }

    FString GetLongPackageName() const
    {
        return IsSoftObjectPtr() ? ((const FSoftObjectPtr*)this)->GetUniqueID().GetLongPackageName() : ((const FAssetPtr*)this)->GetUniqueID().GetLongPackageName();
    }

private:
    static bool IsSoftObjectPtr()
    {
        auto& Version = FEngineVersion::Current();

        return Version.GetMajor() > 4 || Version.GetMinor() >= 18;
    }
};

template<class T>
class TArray<TAssetPtr<T>, FDefaultAllocator> : public TRuntimeSizedArray<TAssetPtr<T>>
{
};
