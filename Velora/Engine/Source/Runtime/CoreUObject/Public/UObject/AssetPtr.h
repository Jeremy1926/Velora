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
        return !Get() && ObjectID.IsValid();
    }

    FORCEINLINE bool IsNull() const
    {
        return !IsValid() && !ObjectID.IsValid();
    }

    FORCEINLINE const TObjectID& GetUniqueID() const
    {
        return ObjectID;
    }

    UObject* LoadSynchronous() const
    {
        UObject* Asset = Get();

        if (IsPending())
            Asset = GetUniqueID().TryLoad();

        return Asset;
    }

private:
    mutable FWeakObjectPtr WeakPtr;
    mutable int32 TagAtLastTest;
    TObjectID ObjectID;
};

struct FAssetPtr : public TPersistentObjectPtr<FStringAssetReference>
{
};

struct FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath>
{
};

template<class T>
class TAssetPtr
{
public:
    static int32 GetTypeSize()
    {
        return IsSoftObjectPtr() ? (int32)sizeof(FSoftObjectPtr) : (int32)sizeof(FAssetPtr);
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
