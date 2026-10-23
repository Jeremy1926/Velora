#pragma once

#include "HAL/Platform.h"
#include <algorithm>
#include <cmath>

template <class T, class PREDICATE_CLASS>
void Sort(T** First, const int32 Num, const PREDICATE_CLASS& Predicate)
{
    std::sort(First, First + Num, [&Predicate](T* A, T* B) { return Predicate(*A, *B); });
}

namespace AlgoImpl
{
    template<typename T, typename PredicateType>
    void HeapSiftDown(T* Heap, int32 Index, const int32 Count, const PredicateType& Predicate)
    {
        while (Index * 2 + 1 < Count)
        {
            const int32 LeftChildIndex = Index * 2 + 1;
            const int32 RightChildIndex = LeftChildIndex + 1;

            int32 MinChildIndex = LeftChildIndex;

            if (RightChildIndex < Count)
                MinChildIndex = Predicate(Heap[RightChildIndex], Heap[LeftChildIndex]) ? LeftChildIndex : RightChildIndex;

            if (!Predicate(Heap[Index], Heap[MinChildIndex]))
                break;

            std::swap(Heap[Index], Heap[MinChildIndex]);
            Index = MinChildIndex;
        }
    }

    template<typename T, typename PredicateType>
    void HeapSortInternal(T* First, int32 Num, const PredicateType& Predicate)
    {
        for (int32 Index = (Num - 2) / 2; Index >= 0; Index--)
            HeapSiftDown(First, Index, Num, Predicate);

        for (int32 Index = Num - 1; Index > 0; Index--)
        {
            std::swap(First[0], First[Index]);

            HeapSiftDown(First, 0, Index, Predicate);
        }
    }

    template<typename T, typename PredicateType>
    void IntroSortInternal(T* First, int32 Num, const PredicateType& Predicate)
    {
        struct FStack
        {
            T* Min;
            T* Max;
            uint32 MaxDepth;
        };

        if (Num < 2)
            return;

        FStack RecursionStack[32] = { { First, First + Num - 1, (uint32)(logf((float)Num) * 2.f) } }, Current, Inner;

        for (FStack* StackTop = RecursionStack; StackTop >= RecursionStack; --StackTop)
        {
            Current = *StackTop;

        Loop:
            const int32 Count = (int32)(Current.Max - Current.Min + 1);

            if (Current.MaxDepth == 0)
            {
                HeapSortInternal(Current.Min, Count, Predicate);
                continue;
            }

            if (Count <= 8)
            {
                while (Current.Max > Current.Min)
                {
                    T* Max = Current.Min;

                    for (T* Item = Current.Min + 1; Item <= Current.Max; Item++)
                    {
                        if (Predicate(*Max, *Item))
                            Max = Item;
                    }

                    std::swap(*Max, *Current.Max--);
                }
            }
            else
            {
                std::swap(Current.Min[Count / 2], Current.Min[0]);

                Inner.Min = Current.Min;
                Inner.Max = Current.Max + 1;

                for (;;)
                {
                    while (++Inner.Min <= Current.Max && !Predicate(*Current.Min, *Inner.Min));
                    while (--Inner.Max > Current.Min && !Predicate(*Inner.Max, *Current.Min));

                    if (Inner.Min > Inner.Max)
                        break;

                    std::swap(*Inner.Min, *Inner.Max);
                }

                std::swap(*Current.Min, *Inner.Max);

                --Current.MaxDepth;

                if (Inner.Max - 1 - Current.Min >= Current.Max - Inner.Min)
                {
                    if (Current.Min + 1 < Inner.Max)
                    {
                        StackTop->Min = Current.Min;
                        StackTop->Max = Inner.Max - 1;
                        StackTop->MaxDepth = Current.MaxDepth;
                        StackTop++;
                    }

                    if (Current.Max > Inner.Min)
                    {
                        Current.Min = Inner.Min;
                        goto Loop;
                    }
                }
                else
                {
                    if (Current.Max > Inner.Min)
                    {
                        StackTop->Min = Inner.Min;
                        StackTop->Max = Current.Max;
                        StackTop->MaxDepth = Current.MaxDepth;
                        StackTop++;
                    }

                    if (Current.Min + 1 < Inner.Max)
                    {
                        Current.Max = Inner.Max - 1;
                        goto Loop;
                    }
                }
            }
        }
    }
}
