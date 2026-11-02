#pragma once

#include "UObject/AssetPtr.h"

class UObject;

template<class T = UObject>
class TSoftObjectPtr
{
public:
    static FORCEINLINE int32 GetTypeSize()
    {
        return FSoftObjectPtr::GetTypeSize();
    }

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
    static FORCEINLINE int32 GetTypeSize()
    {
        return FSoftObjectPtr::GetTypeSize();
    }

    UClass* LoadSynchronous() const
    {
        return Cast<UClass>(SoftObjectPtr.LoadSynchronous());
    }

private:
    FSoftObjectPtr SoftObjectPtr;
};

template<class T>
class TArray<TSoftObjectPtr<T>, FDefaultAllocator> : public TRuntimeSizedArray<TSoftObjectPtr<T>>
{
public:
    void Add(const TSoftObjectPtr<T>& Item)
    {
        new(&TRuntimeSizedArray<TSoftObjectPtr<T>>::operator[](this->AddUninitialized(1))) TSoftObjectPtr<T>(Item);
    }
};
