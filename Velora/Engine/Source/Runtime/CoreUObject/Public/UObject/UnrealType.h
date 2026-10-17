#pragma once

#include "HAL/Platform.h"
#include "Containers/EnumAsByte.h"
#include "UObject/Class.h"
#include "UObject/CoreNetTypes.h"

class UProperty : public UField
{
public:
    int32 ArrayDim;
    int32 ElementSize;
    uint64 PropertyFlags;
    uint16 RepIndex;

private:
    TEnumAsByte<ELifetimeCondition> BlueprintReplicationCondition;
    int32 Offset_Internal;

    FORCEINLINE void* ContainerVoidPtrToValuePtrInternal(void* ContainerPtr, int32 ArrayIndex) const
    {
        return (uint8*)ContainerPtr + Offset_Internal + ElementSize * ArrayIndex;
    }

public:
    template<typename ValueType>
    FORCEINLINE ValueType* ContainerPtrToValuePtr(void* ContainerPtr, int32 ArrayIndex = 0) const
    {
        return (ValueType*)ContainerVoidPtrToValuePtrInternal(ContainerPtr, ArrayIndex);
    }

    template<typename ValueType>
    FORCEINLINE ValueType const* ContainerPtrToValuePtr(void const* ContainerPtr, int32 ArrayIndex = 0) const
    {
        return ContainerPtrToValuePtr<ValueType>((void*)ContainerPtr, ArrayIndex);
    }
};
