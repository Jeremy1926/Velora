#pragma once

#include <cstring>
#include "HAL/Platform.h"
#include "Templates/RemoveReference.h"

template <typename T>
FORCEINLINE typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
{
    return (typename TRemoveReference<T>::Type&&)Obj;
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type& Obj)
{
    return (T&&)Obj;
}

template <typename T>
FORCEINLINE T&& Forward(typename TRemoveReference<T>::Type&& Obj)
{
    return (T&&)Obj;
}

template<typename ValueType>
struct TGameLayout;

template<typename ValueType>
class TGameValue
{
public:
    FORCEINLINE TGameValue()
    {
        memset(Bytes, 0, sizeof(Bytes));
    }

    FORCEINLINE TGameValue(const ValueType& Value)
        : TGameValue()
    {
        TGameLayout<ValueType>::Store(Bytes, Value);
    }

    FORCEINLINE ValueType Get() const
    {
        return TGameLayout<ValueType>::Load(Bytes);
    }

private:
    alignas(alignof(typename TGameLayout<ValueType>::FStorage)) uint8 Bytes[sizeof(typename TGameLayout<ValueType>::FStorage)];
};
