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

public:
    FName RepNotifyFunc;
    UProperty* PropertyLinkNext;
    UProperty* NextRef;
    UProperty* DestructorLinkNext;
    UProperty* PostConstructLinkNext;

    FORCEINLINE int32 GetOffset_ForInternal() const
    {
        return Offset_Internal;
    }

private:
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

class UBoolProperty : public UProperty
{
public:
    FORCEINLINE bool GetPropertyValue(void const* A) const
    {
        uint8* ByteValue = (uint8*)A + ByteOffset;

        return !!(*ByteValue & FieldMask);
    }

    FORCEINLINE bool GetPropertyValue_InContainer(void const* A, int32 ArrayIndex = 0) const
    {
        return GetPropertyValue(ContainerPtrToValuePtr<void>(A, ArrayIndex));
    }

    FORCEINLINE uint8 GetByteOffset() const
    {
        return ByteOffset;
    }

    FORCEINLINE uint8 GetFieldMask() const
    {
        return FieldMask;
    }

private:
    uint8 FieldSize;
    uint8 ByteOffset;
    uint8 ByteMask;
    uint8 FieldMask;
};
