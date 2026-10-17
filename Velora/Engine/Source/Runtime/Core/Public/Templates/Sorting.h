#pragma once

#include "HAL/Platform.h"
#include <algorithm>

template <class T, class PREDICATE_CLASS>
void Sort(T** First, const int32 Num, const PREDICATE_CLASS& Predicate)
{
    std::sort(First, First + Num, [&Predicate](T* A, T* B) { return Predicate(*A, *B); });
}
