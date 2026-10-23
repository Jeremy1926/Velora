#pragma once

#include "HAL/Platform.h"
#include "Containers/EnumAsByte.h"
#include "UObject/Class.h"
#include "UObject/Field.h"
#include "UObject/CoreNetTypes.h"

class UProperty : public UField
{
public:
    FORCEINLINE int32 GetSize() const
    {
        return GetProperty()->ArrayDim * GetProperty()->ElementSize;
    }

    FORCEINLINE int32 GetOffset_ForInternal() const
    {
        return GetProperty()->Offset_Internal;
    }

private:
    FORCEINLINE void* ContainerVoidPtrToValuePtrInternal(void* ContainerPtr, int32 ArrayIndex) const
    {
        return (uint8*)ContainerPtr + GetProperty()->Offset_Internal + GetProperty()->ElementSize * ArrayIndex;
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

    template<typename ValueType>
    FORCEINLINE ValueType GetPropertyValue_InContainer(void const* ContainerPtr, int32 ArrayIndex = 0) const
    {
        return TGameLayout<ValueType>::Load(ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex));
    }

    template<typename ValueType>
    FORCEINLINE void SetPropertyValue_InContainer(void* ContainerPtr, const ValueType& Value, int32 ArrayIndex = 0) const
    {
        TGameLayout<ValueType>::Store(ContainerPtrToValuePtr<void>(ContainerPtr, ArrayIndex), Value);
    }

protected:
    FORCEINLINE const UProperty* GetProperty() const
    {
        return FField::StaticClass() ? (const UProperty*)((const uint8*)this + sizeof(FField) - sizeof(UField)) : this;
    }

private:
    int32 ArrayDim;
    int32 ElementSize;
    uint64 PropertyFlags;
    uint16 RepIndex;
    TEnumAsByte<ELifetimeCondition> BlueprintReplicationCondition;
    int32 Offset_Internal;
    FName RepNotifyFunc;
    UProperty* PropertyLinkNext;
    UProperty* NextRef;
    UProperty* DestructorLinkNext;
    UProperty* PostConstructLinkNext;
};

class UBoolProperty : public UProperty
{
public:
    FORCEINLINE bool GetPropertyValue(void const* A) const
    {
        uint8* ByteValue = (uint8*)A + GetProperty()->ByteOffset;

        return !!(*ByteValue & GetProperty()->FieldMask);
    }

    FORCEINLINE bool GetPropertyValue_InContainer(void const* A, int32 ArrayIndex = 0) const
    {
        return GetPropertyValue(ContainerPtrToValuePtr<void>(A, ArrayIndex));
    }

    FORCEINLINE void SetPropertyValue(void* A, bool Value) const
    {
        uint8* ByteValue = (uint8*)A + GetProperty()->ByteOffset;

        *ByteValue = ((*ByteValue) & ~GetProperty()->FieldMask) | (Value ? GetProperty()->ByteMask : 0);
    }

    FORCEINLINE void SetPropertyValue_InContainer(void* A, bool Value, int32 ArrayIndex = 0) const
    {
        SetPropertyValue(ContainerPtrToValuePtr<void>(A, ArrayIndex), Value);
    }

    FORCEINLINE uint8 GetByteOffset() const
    {
        return GetProperty()->ByteOffset;
    }

    FORCEINLINE uint8 GetFieldMask() const
    {
        return GetProperty()->FieldMask;
    }

private:
    FORCEINLINE const UBoolProperty* GetProperty() const
    {
        return (const UBoolProperty*)UProperty::GetProperty();
    }

    uint8 FieldSize;
    uint8 ByteOffset;
    uint8 ByteMask;
    uint8 FieldMask;
};

class UArrayProperty : public UProperty
{
public:
    FORCEINLINE UProperty* GetInner() const
    {
        return GetProperty()->Inner;
    }

private:
    FORCEINLINE const UArrayProperty* GetProperty() const
    {
        return (const UArrayProperty*)UProperty::GetProperty();
    }

    UProperty* Inner;
};
