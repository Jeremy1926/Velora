#pragma once

#include "UObject/AssetPtr.h"

class UObject;

template<class T = UObject>
class TSoftObjectPtr
{
public:
    FORCEINLINE bool IsNull() const
    {
        return SoftObjectPtr.IsNull();
    }

    T* LoadSynchronous() const
    {
        return Cast<T>(SoftObjectPtr.LoadSynchronous());
    }

    FString ToString() const
    {
        return SoftObjectPtr.GetUniqueID().ToString();
    }

private:
    FSoftObjectPtr SoftObjectPtr;
};

template<class TClass = UObject>
class TSoftClassPtr
{
public:
    UClass* LoadSynchronous() const
    {
        return Cast<UClass>(SoftObjectPtr.LoadSynchronous());
    }

private:
    FSoftObjectPtr SoftObjectPtr;
};
