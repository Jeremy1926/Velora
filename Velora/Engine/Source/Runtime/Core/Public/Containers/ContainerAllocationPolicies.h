#pragma once

#include "HAL/Platform.h"
#include "HAL/UnrealMemory.h"

FORCEINLINE int32 DefaultCalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = 0)
{
    int32 Retval;

    const SIZE_T CurrentSlackElements = NumAllocatedElements - NumElements;
    const SIZE_T CurrentSlackBytes = (NumAllocatedElements - NumElements) * BytesPerElement;
    const bool bTooManySlackBytes = CurrentSlackBytes >= 16384;
    const bool bTooManySlackElements = 3 * NumElements < 2 * NumAllocatedElements;

    if ((bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements))
    {
        Retval = NumElements;

        if (Retval > 0 && bAllowQuantize)
            Retval = (int32)(FMemory::QuantizeSize(Retval * BytesPerElement, Alignment) / BytesPerElement);
    }
    else
    {
        Retval = NumAllocatedElements;
    }

    return Retval;
}

FORCEINLINE int32 DefaultCalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = 0)
{
    int32 Retval;
    SIZE_T Grow = 4;

    if (NumAllocatedElements || SIZE_T(NumElements) > Grow)
        Grow = SIZE_T(NumElements) + 3 * SIZE_T(NumElements) / 8 + 16;

    if (bAllowQuantize)
        Retval = (int32)(FMemory::QuantizeSize(Grow * BytesPerElement, Alignment) / BytesPerElement);
    else
        Retval = (int32)Grow;

    if (NumElements > Retval)
        Retval = 0x7FFFFFFF;

    return Retval;
}

FORCEINLINE int32 DefaultCalculateSlackReserve(int32 NumElements, SIZE_T BytesPerElement, bool bAllowQuantize, uint32 Alignment = 0)
{
    int32 Retval = NumElements;

    if (bAllowQuantize)
    {
        Retval = (int32)(FMemory::QuantizeSize(SIZE_T(Retval) * SIZE_T(BytesPerElement), Alignment) / BytesPerElement);

        if (NumElements > Retval)
            Retval = 0x7FFFFFFF;
    }

    return Retval;
}

struct FScriptContainerElement
{
};

class FHeapAllocator
{
public:
    enum { NeedsElementType = false };

    class ForAnyElementType
    {
    public:
        ForAnyElementType()
            : Data(nullptr)
        {
        }

        FORCEINLINE void MoveToEmpty(ForAnyElementType& Other)
        {
            if (Data)
            {
                FMemory::Free(Data);
            }

            Data = Other.Data;
            Other.Data = nullptr;
        }

        FORCEINLINE ~ForAnyElementType()
        {
            if (Data)
            {
                FMemory::Free(Data);
            }
        }

        FORCEINLINE FScriptContainerElement* GetAllocation() const
        {
            return Data;
        }

        FORCEINLINE void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
        {
            if (Data || NumElements)
                Data = (FScriptContainerElement*)FMemory::Realloc(Data, NumElements * NumBytesPerElement);
        }

        FORCEINLINE int32 CalculateSlackReserve(int32 NumElements, SIZE_T NumBytesPerElement) const
        {
            return DefaultCalculateSlackReserve(NumElements, NumBytesPerElement, true);
        }

        FORCEINLINE int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
        {
            return DefaultCalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement, true);
        }

        FORCEINLINE int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
        {
            return DefaultCalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement, true);
        }

    private:
        ForAnyElementType(const ForAnyElementType&);
        ForAnyElementType& operator=(const ForAnyElementType&);

        FScriptContainerElement* Data;
    };

    template<typename ElementType>
    class ForElementType : public ForAnyElementType
    {
    public:
        FORCEINLINE ElementType* GetAllocation() const
        {
            return (ElementType*)ForAnyElementType::GetAllocation();
        }
    };
};

class FDefaultAllocator : public FHeapAllocator
{
public:
    typedef FHeapAllocator Typedef;
};

template<uint32 NumInlineElements, typename SecondaryAllocator = FDefaultAllocator>
class TInlineAllocator
{
public:
    enum { NeedsElementType = true };

    template<typename ElementType>
    class ForElementType
    {
    public:
        ForElementType()
        {
        }

        FORCEINLINE ElementType* GetAllocation() const
        {
            return SecondaryData.GetAllocation() ? SecondaryData.GetAllocation() : GetInlineElements();
        }

        void MoveToEmpty(ForElementType& Other)
        {
            if (!Other.SecondaryData.GetAllocation())
                memmove((void*)InlineData, (void*)Other.InlineData, sizeof(InlineData));

            SecondaryData.MoveToEmpty(Other.SecondaryData);
        }

        void ResizeAllocation(int32 PreviousNumElements, int32 NumElements, SIZE_T NumBytesPerElement)
        {
            if (NumElements <= NumInlineElements)
            {
                if (SecondaryData.GetAllocation())
                {
                    memmove((void*)InlineData, (void*)SecondaryData.GetAllocation(), PreviousNumElements * sizeof(ElementType));
                    SecondaryData.ResizeAllocation(0, 0, NumBytesPerElement);
                }
            }
            else
            {
                if (!SecondaryData.GetAllocation())
                {
                    SecondaryData.ResizeAllocation(0, NumElements, NumBytesPerElement);
                    memmove((void*)SecondaryData.GetAllocation(), (void*)GetInlineElements(), PreviousNumElements * sizeof(ElementType));
                }
                else
                {
                    SecondaryData.ResizeAllocation(PreviousNumElements, NumElements, NumBytesPerElement);
                }
            }
        }

        FORCEINLINE int32 CalculateSlackReserve(int32 NumElements, SIZE_T NumBytesPerElement) const
        {
            return NumElements <= NumInlineElements ? NumInlineElements : SecondaryData.CalculateSlackReserve(NumElements, NumBytesPerElement);
        }

        FORCEINLINE int32 CalculateSlackShrink(int32 NumElements, int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
        {
            return NumElements <= NumInlineElements ? NumInlineElements : SecondaryData.CalculateSlackShrink(NumElements, NumAllocatedElements, NumBytesPerElement);
        }

        FORCEINLINE int32 CalculateSlackGrow(int32 NumElements, int32 NumAllocatedElements, SIZE_T NumBytesPerElement) const
        {
            return NumElements <= NumInlineElements ? NumInlineElements : SecondaryData.CalculateSlackGrow(NumElements, NumAllocatedElements, NumBytesPerElement);
        }

    private:
        ForElementType(const ForElementType&);
        ForElementType& operator=(const ForElementType&);

        FORCEINLINE ElementType* GetInlineElements() const
        {
            return (ElementType*)InlineData;
        }

        alignas(ElementType) uint8 InlineData[NumInlineElements * sizeof(ElementType)];
        typename SecondaryAllocator::template ForElementType<ElementType> SecondaryData;
    };

    typedef void ForAnyElementType;
};

class FDefaultBitArrayAllocator : public TInlineAllocator<4>
{
};
