#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/BitArray.h"
#include "Templates/TypeCompatibleBytes.h"

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

    void RemoveAt(int32 Index, int32 Count = 1)
    {
        for (int32 It = Index, ItCount = Count; ItCount; ++It, --ItCount)
        {
            ((ElementType&)GetData(It).ElementData).~ElementType();
        }

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
