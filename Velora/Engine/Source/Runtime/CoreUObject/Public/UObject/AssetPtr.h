#pragma once

#include "UObject/WeakObjectPtr.h"
#include "Misc/StringAssetReference.h"
#include "Templates/Casts.h"

struct FAssetPtr
{
public:
    FORCEINLINE UObject* Get() const
    {
        return WeakPtr.Get();
    }

    FORCEINLINE bool IsPending() const
    {
        return !Get() && ObjectID.IsValid();
    }

    FORCEINLINE const FStringAssetReference& GetUniqueID() const
    {
        return ObjectID;
    }

private:
    mutable FWeakObjectPtr WeakPtr;
    mutable int32 TagAtLastTest;
    FStringAssetReference ObjectID;
};

template<class T>
class TAssetPtr
{
public:
    T* LoadSynchronous() const
    {
        UObject* Asset = AssetPtr.Get();

        if (AssetPtr.IsPending())
            Asset = AssetPtr.GetUniqueID().TryLoad();

        return Cast<T>(Asset);
    }

private:
    FAssetPtr AssetPtr;
};
