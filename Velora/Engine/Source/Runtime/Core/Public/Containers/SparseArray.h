#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/BitArray.h"
#include "Templates/TypeCompatibleBytes.h"

struct FSparseArrayAllocationInfo
{
    int32 Index;
    void* Pointer;
};

FORCEINLINE void* operator new(size_t Size, const FSparseArrayAllocationInfo& Allocation)
{
    return Allocation.Pointer;
}

template<typename ElementType>
union TSparseArrayElementOrFreeListLink
{
    ElementType ElementData;

    struct
    {
        int32 PrevFreeIndex;
        int32 NextFreeIndex;
    };
};

template<typename InElementType>
class TSparseArray
{
public:
    typedef InElementType ElementType;

    FORCEINLINE TSparseArray()
        : FirstFreeIndex(-1)
        , NumFreeIndices(0)
    {
    }

    FORCEINLINE ~TSparseArray()
    {
        Empty();
    }

    FORCEINLINE int32 Num() const
    {
        return Data.Num() - NumFreeIndices;
    }

    FORCEINLINE int32 GetMaxIndex() const
    {
        return Data.Num();
    }

    FORCEINLINE bool IsAllocated(int32 Index) const
    {
        return AllocationFlags[Index];
    }

    FORCEINLINE ElementType& operator[](int32 Index)
    {
        return *(ElementType*)&GetData(Index).ElementData;
    }

    FORCEINLINE const ElementType& operator[](int32 Index) const
    {
        return *(const ElementType*)&GetData(Index).ElementData;
    }

    FSparseArrayAllocationInfo AllocateIndex(int32 Index)
    {
        AllocationFlags.SetBit(Index, true);

        FSparseArrayAllocationInfo Result;
        Result.Index = Index;
        Result.Pointer = &GetData(Result.Index).ElementData;

        return Result;
    }

    FSparseArrayAllocationInfo AddUninitialized()
    {
        int32 Index;

        if (NumFreeIndices)
        {
            Index = FirstFreeIndex;
            FirstFreeIndex = GetData(FirstFreeIndex).NextFreeIndex;
            --NumFreeIndices;

            if (NumFreeIndices)
            {
                GetData(FirstFreeIndex).PrevFreeIndex = -1;
            }
        }
        else
        {
            Index = Data.AddUninitialized(1);
            AllocationFlags.Add(false);
        }

        return AllocateIndex(Index);
    }

    void RemoveAt(int32 Index, int32 Count = 1)
    {
        for (int32 It = Index, ItCount = Count; ItCount; ++It, --ItCount)
        {
            ((ElementType&)GetData(It).ElementData).~ElementType();
        }

        RemoveAtUninitialized(Index, Count);
    }

    void RemoveAtUninitialized(int32 Index, int32 Count = 1)
    {
        for (; Count; --Count)
        {
            if (NumFreeIndices)
            {
                GetData(FirstFreeIndex).PrevFreeIndex = Index;
            }

            auto& IndexData = GetData(Index);
            IndexData.PrevFreeIndex = -1;
            IndexData.NextFreeIndex = NumFreeIndices > 0 ? FirstFreeIndex : -1;
            FirstFreeIndex = Index;
            ++NumFreeIndices;
            AllocationFlags.SetBit(Index, false);

            ++Index;
        }
    }

    void Empty()
    {
        for (int32 Index = 0; Index < GetMaxIndex(); Index++)
        {
            if (IsAllocated(Index))
            {
                ((ElementType&)GetData(Index).ElementData).~ElementType();
            }
        }

        Data.Empty();
        FirstFreeIndex = -1;
        NumFreeIndices = 0;
        AllocationFlags.Empty();
    }

    void Reset()
    {
        for (int32 Index = 0; Index < GetMaxIndex(); Index++)
        {
            if (IsAllocated(Index))
            {
                ((ElementType&)GetData(Index).ElementData).~ElementType();
            }
        }

        Data.Reset();
        FirstFreeIndex = -1;
        NumFreeIndices = 0;
        AllocationFlags.Reset();
    }

private:
    typedef TSparseArrayElementOrFreeListLink<TAlignedBytes<sizeof(ElementType), alignof(ElementType)>> FElementOrFreeListLink;

    FORCEINLINE FElementOrFreeListLink& GetData(int32 Index)
    {
        return Data.GetData()[Index];
    }

    FORCEINLINE const FElementOrFreeListLink& GetData(int32 Index) const
    {
        return Data.GetData()[Index];
    }

    TArray<FElementOrFreeListLink> Data;
    TBitArray<> AllocationFlags;
    int32 FirstFreeIndex;
    int32 NumFreeIndices;
};
