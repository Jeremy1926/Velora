#pragma once

#include "HAL/Platform.h"
#include "Containers/Map.h"

class UObject;
struct FWeakObjectPtr;

template<class T, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr : private TWeakObjectPtrBase
{
public:
    FORCEINLINE TWeakObjectPtr()
    {
    }

    FORCEINLINE TWeakObjectPtr(const T* Object)
        : TWeakObjectPtrBase((const UObject*)Object)
    {
    }

    FORCEINLINE T* Get() const
    {
        return (T*)TWeakObjectPtrBase::Get();
    }

    FORCEINLINE bool HasSameIndexAndSerialNumber(const TWeakObjectPtr& Other) const
    {
        return static_cast<const TWeakObjectPtrBase&>(*this).HasSameIndexAndSerialNumber(static_cast<const TWeakObjectPtrBase&>(Other));
    }

    FORCEINLINE bool operator==(const TWeakObjectPtr& Other) const
    {
        return static_cast<const TWeakObjectPtrBase&>(*this) == static_cast<const TWeakObjectPtrBase&>(Other);
    }

    friend uint32 GetTypeHash(const TWeakObjectPtr& WeakObjectPtr)
    {
        return GetTypeHash(static_cast<const TWeakObjectPtrBase&>(WeakObjectPtr));
    }
};

template<typename KeyType, typename ValueType>
struct TWeakObjectPtrMapKeyFuncs : public TDefaultMapKeyFuncs<KeyType, ValueType>
{
    typedef typename TDefaultMapKeyFuncs<KeyType, ValueType>::KeyInitType KeyInitType;

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A.HasSameIndexAndSerialNumber(B);
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
    {
        return GetTypeHash(Key);
    }
};
