#pragma once

#include "HAL/Platform.h"
#include "Templates/RemoveReference.h"

template <typename T>
FORCEINLINE typename TRemoveReference<T>::Type&& MoveTemp(T&& Obj)
{
    return (typename TRemoveReference<T>::Type&&)Obj;
}
