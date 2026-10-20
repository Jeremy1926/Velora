#pragma once

#include "HAL/Platform.h"
#include "Containers/ContainersFwd.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Templates/ChooseClass.h"
#include <new>

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

    FORCEINLINE TArray(const TArray& Other)
    {
        CopyToEmpty(Other.GetData(), Other.Num(), 0, 0);
    }

    FORCEINLINE TArray(TArray&& Other)
    {
        MoveOrCopy(*this, Other, 0);
    }

    FORCEINLINE ~TArray()
    {
        DestructItems(GetData(), ArrayNum);
    }

    FORCEINLINE ElementType* GetData() const
    {
        return (ElementType*)AllocatorInstance.GetAllocation();
    }

    FORCEINLINE bool IsValidIndex(int32 Index) const
    {
        return Index >= 0 && Index < ArrayNum;
    }

    FORCEINLINE int32 Num() const
    {
        return ArrayNum;
    }

    FORCEINLINE int32 Max() const
    {
        return ArrayMax;
    }

    FORCEINLINE ElementType& operator[](int32 Index)
    {
        return GetData()[Index];
    }

    FORCEINLINE const ElementType& operator[](int32 Index) const
    {
        return GetData()[Index];
    }

    FORCEINLINE ElementType* begin()
    {
        return GetData();
    }

    FORCEINLINE const ElementType* begin() const
    {
        return GetData();
    }

    FORCEINLINE ElementType* end()
    {
        return GetData() + Num();
    }

    FORCEINLINE const ElementType* end() const
    {
        return GetData() + Num();
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

    FORCEINLINE int32 Add(const ElementType& Item)
    {
        const int32 Index = AddUninitialized(1);
        new(GetData() + Index) ElementType(Item);

        return Index;
    }

    FORCEINLINE void Reserve(int32 Number)
    {
        if (Number > ArrayMax)
        {
            ResizeTo(Number);
        }
    }

    void Reset(int32 NewSize = 0)
    {
        if (NewSize <= ArrayMax)
        {
            DestructItems(GetData(), ArrayNum);
            ArrayNum = 0;
        }
        else
        {
            Empty(NewSize);
        }
    }

    void Empty(int32 Slack = 0)
    {
        DestructItems(GetData(), ArrayNum);
        ArrayNum = 0;

        if (ArrayMax != Slack)
        {
            ResizeTo(Slack);
        }
    }

    void RemoveAt(int32 Index, int32 Count = 1, bool bAllowShrinking = true)
    {
        if (Count)
        {
            DestructItems(GetData() + Index, Count);

            const int32 NumToMove = ArrayNum - Index - Count;

            if (NumToMove)
            {
                memmove(GetData() + Index, GetData() + Index + Count, NumToMove * sizeof(ElementType));
            }

            ArrayNum -= Count;

            if (bAllowShrinking)
            {
                ResizeShrink();
            }
        }
    }

    FORCEINLINE bool Contains(const ElementType& Item) const
    {
        for (const ElementType* Data = GetData(), *DataEnd = Data + ArrayNum; Data != DataEnd; ++Data)
        {
            if (*Data == Item)
                return true;
        }

        return false;
    }

    void Append(const TArray& Source)
    {
        for (const ElementType& Item : Source)
            Add(Item);
    }

    int32 RemoveSingle(const ElementType& Item)
    {
        for (int32 Index = 0; Index < ArrayNum; Index++)
        {
            auto RemovePtr = GetData() + Index;

            if (*RemovePtr == Item)
            {
                DestructItems(RemovePtr, 1);
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

    void CopyToEmpty(const ElementType* OtherData, int32 OtherNum, int32 PrevMax, int32 ExtraSlack)
    {
        ArrayNum = OtherNum;

        if (OtherNum || ExtraSlack || PrevMax)
        {
            ResizeForCopy(OtherNum + ExtraSlack, PrevMax);

            for (int32 Index = 0; Index < OtherNum; Index++)
            {
                new(GetData() + Index) ElementType(OtherData[Index]);
            }
        }
        else
        {
            ArrayMax = 0;
        }
    }

    static FORCEINLINE void DestructItems(ElementType* Element, int32 Count)
    {
        while (Count)
        {
            Element->~ElementType();
            ++Element;
            --Count;
        }
    }

    FORCENOINLINE void ResizeGrow(int32 OldNum)
    {
        ArrayMax = AllocatorInstance.CalculateSlackGrow(ArrayNum, ArrayMax, sizeof(ElementType));
        AllocatorInstance.ResizeAllocation(OldNum, ArrayMax, sizeof(ElementType));
    }

    FORCEINLINE void ResizeShrink()
    {
        const int32 NewArrayMax = AllocatorInstance.CalculateSlackShrink(ArrayNum, ArrayMax, sizeof(ElementType));

        if (NewArrayMax != ArrayMax)
        {
            ArrayMax = NewArrayMax;
            AllocatorInstance.ResizeAllocation(ArrayNum, ArrayMax, sizeof(ElementType));
        }
    }

    FORCENOINLINE void ResizeTo(int32 NewMax)
    {
        if (NewMax)
        {
            NewMax = AllocatorInstance.CalculateSlackReserve(NewMax, sizeof(ElementType));
        }

        if (NewMax != ArrayMax)
        {
            ArrayMax = NewMax;
            AllocatorInstance.ResizeAllocation(ArrayNum, ArrayMax, sizeof(ElementType));
        }
    }

    FORCENOINLINE void ResizeForCopy(int32 NewMax, int32 PrevMax)
    {
        if (NewMax)
        {
            NewMax = AllocatorInstance.CalculateSlackReserve(NewMax, sizeof(ElementType));
        }

        if (NewMax != PrevMax)
        {
            AllocatorInstance.ResizeAllocation(0, NewMax, sizeof(ElementType));
        }

        ArrayMax = NewMax;
    }
};

template <typename T, typename AllocatorType>
void* operator new(size_t Size, TArray<T, AllocatorType>& Array)
{
    const int32 Index = Array.AddUninitialized(1);

    return &Array[Index];
}

template<typename InElementType>
class TRuntimeSizedArray
{
public:
    typedef InElementType ElementType;

    class TIterator
    {
    public:
        FORCEINLINE TIterator(uint8* InElement)
            : Element(InElement)
        {
        }

        FORCEINLINE ElementType& operator*() const
        {
            return *(ElementType*)Element;
        }

        FORCEINLINE TIterator& operator++()
        {
            Element += ElementType::GetTypeSize();

            return *this;
        }

        FORCEINLINE bool operator!=(const TIterator& Other) const
        {
            return Element != Other.Element;
        }

    private:
        uint8* Element;
    };

    FORCEINLINE TRuntimeSizedArray()
        : ArrayNum(0)
        , ArrayMax(0)
    {
    }

    ~TRuntimeSizedArray()
    {
        for (auto& Element : *this)
            Element.~ElementType();
    }

    FORCEINLINE int32 Num() const
    {
        return ArrayNum;
    }

    FORCEINLINE ElementType& operator[](int32 Index) const
    {
        return *(ElementType*)((uint8*)AllocatorInstance.GetAllocation() + Index * ElementType::GetTypeSize());
    }

    FORCEINLINE TIterator begin() const
    {
        return TIterator((uint8*)AllocatorInstance.GetAllocation());
    }

    FORCEINLINE TIterator end() const
    {
        return TIterator((uint8*)AllocatorInstance.GetAllocation() + ArrayNum * ElementType::GetTypeSize());
    }

    int32 AddUninitialized(int32 Count = 1)
    {
        const int32 OldNum = ArrayNum;

        if ((ArrayNum += Count) > ArrayMax)
        {
            ArrayMax = AllocatorInstance.CalculateSlackGrow(ArrayNum, ArrayMax, ElementType::GetTypeSize());
            AllocatorInstance.ResizeAllocation(OldNum, ArrayMax, ElementType::GetTypeSize());
        }

        return OldNum;
    }

    void RemoveAt(int32 Index, int32 Count = 1, bool bAllowShrinking = true)
    {
        if (Count)
        {
            for (int32 RemoveIndex = Index; RemoveIndex < Index + Count; RemoveIndex++)
                (*this)[RemoveIndex].~ElementType();

            const int32 NumToMove = ArrayNum - Index - Count;

            if (NumToMove)
            {
                memmove(&(*this)[Index], &(*this)[Index + Count], NumToMove * ElementType::GetTypeSize());
            }

            ArrayNum -= Count;

            if (bAllowShrinking)
            {
                const int32 NewArrayMax = AllocatorInstance.CalculateSlackShrink(ArrayNum, ArrayMax, ElementType::GetTypeSize());

                if (NewArrayMax != ArrayMax)
                {
                    ArrayMax = NewArrayMax;
                    AllocatorInstance.ResizeAllocation(ArrayNum, ArrayMax, ElementType::GetTypeSize());
                }
            }
        }
    }

private:
    FDefaultAllocator::ForAnyElementType AllocatorInstance;
    int32 ArrayNum;
    int32 ArrayMax;
};
