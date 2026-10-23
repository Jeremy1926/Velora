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

private:
    ElementType* DataPtr;
    SizeType ArrayNum;
};
