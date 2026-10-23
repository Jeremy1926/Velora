#pragma once

#include "HAL/Platform.h"

template<typename InElementType, typename InSizeType = int32>
class TArrayView
{
public:
    typedef InElementType ElementType;
    typedef InSizeType SizeType;

    TArrayView(ElementType* InData, SizeType InCount)
        : DataPtr(InData)
        , ArrayNum(InCount)
    {
    }

    FORCEINLINE ElementType* GetData() const
    {
        return DataPtr;
    }

    FORCEINLINE SizeType Num() const
    {
        return ArrayNum;
    }

    FORCEINLINE ElementType& operator[](SizeType Index) const
    {
        return DataPtr[Index];
    }

private:
    ElementType* DataPtr;
    SizeType ArrayNum;
};
