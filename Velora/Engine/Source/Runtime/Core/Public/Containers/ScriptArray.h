#pragma once

#include "HAL/Platform.h"
#include "Containers/ContainerAllocationPolicies.h"

template<typename AllocatorType>
class TScriptArray : protected AllocatorType::ForAnyElementType
{
public:
    FORCEINLINE void* GetData()
    {
        return this->GetAllocation();
    }

    FORCEINLINE int32 Num() const
    {
        return ArrayNum;
    }

    int32 Add(int32 Count, int32 NumBytesPerElement)
    {
        const int32 OldNum = ArrayNum;

        if ((ArrayNum += Count) > ArrayMax)
        {
            ResizeGrow(OldNum, NumBytesPerElement);
        }

        return OldNum;
    }

    void Remove(int32 Index, int32 Count, int32 NumBytesPerElement)
    {
        if (Count)
        {
            int32 NumToMove = ArrayNum - Index - Count;

            if (NumToMove)
            {
                memmove((uint8*)this->GetAllocation() + Index * NumBytesPerElement, (uint8*)this->GetAllocation() + (Index + Count) * NumBytesPerElement, NumToMove * NumBytesPerElement);
            }

            ArrayNum -= Count;

            ResizeShrink(NumBytesPerElement);
        }
    }

protected:
    int32 ArrayNum;
    int32 ArrayMax;

    FORCENOINLINE void ResizeGrow(int32 OldNum, int32 NumBytesPerElement)
    {
        ArrayMax = this->CalculateSlackGrow(ArrayNum, ArrayMax, NumBytesPerElement);
        this->ResizeAllocation(OldNum, ArrayMax, NumBytesPerElement);
    }

    FORCENOINLINE void ResizeShrink(int32 NumBytesPerElement)
    {
        const int32 NewArrayMax = this->CalculateSlackShrink(ArrayNum, ArrayMax, NumBytesPerElement);

        if (NewArrayMax != ArrayMax)
        {
            ArrayMax = NewArrayMax;
            this->ResizeAllocation(ArrayNum, ArrayMax, NumBytesPerElement);
        }
    }
};

class FScriptArray : public TScriptArray<FHeapAllocator>
{
};
