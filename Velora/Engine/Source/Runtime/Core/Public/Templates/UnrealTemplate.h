#pragma once

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
