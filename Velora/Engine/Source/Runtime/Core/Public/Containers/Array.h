#pragma once

#include "HAL/Platform.h"
#include "Containers/ContainersFwd.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Templates/ChooseClass.h"

template<typename InElementType, typename InAllocator>
class TArray
{
public:
    typedef InElementType ElementType;
    typedef InAllocator Allocator;

    FORCEINLINE TArray()
        : ArrayNum(0)
        , ArrayMax(0)
    {
    }

    FORCEINLINE TArray(TArray&& Other)
    {
        MoveOrCopy(*this, Other, 0);
    }

    FORCEINLINE ElementType* GetData() const
    {
        return (ElementType*)AllocatorInstance.GetAllocation();
    }

    FORCEINLINE int32 Num() const
    {
        return ArrayNum;
    }

    FORCEINLINE int32 AddUninitialized(int32 Count = 1)
    {
        const int32 OldNum = ArrayNum;

        if ((ArrayNum += Count) > ArrayMax)
        {
            ResizeGrow(OldNum);
        }

        return OldNum;
    }

    int32 RemoveSingle(const ElementType& Item)
    {
        for (int32 Index = 0; Index < ArrayNum; Index++)
        {
            auto RemovePtr = GetData() + Index;

            if (*RemovePtr == Item)
            {
                RemovePtr->~ElementType();
                memmove(RemovePtr, RemovePtr + 1, (ArrayNum - (Index + 1)) * sizeof(ElementType));
                --ArrayNum;

                return 1;
            }
        }

        return 0;
    }

protected:
    typedef typename TChooseClass<
        Allocator::NeedsElementType,
        typename Allocator::template ForElementType<ElementType>,
        typename Allocator::ForAnyElementType
    >::Result ElementAllocatorType;

    ElementAllocatorType AllocatorInstance;
    int32 ArrayNum;
    int32 ArrayMax;

private:
    template <typename FromArrayType, typename ToArrayType>
    static FORCEINLINE void MoveOrCopy(ToArrayType& ToArray, FromArrayType& FromArray, int32 PrevMax)
    {
        ToArray.AllocatorInstance.MoveToEmpty(FromArray.AllocatorInstance);

        ToArray.ArrayNum = FromArray.ArrayNum;
        ToArray.ArrayMax = FromArray.ArrayMax;
        FromArray.ArrayNum = 0;
        FromArray.ArrayMax = 0;
    }

    FORCENOINLINE void ResizeGrow(int32 OldNum);
};
