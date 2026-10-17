#pragma once

#include "HAL/Platform.h"
#include "Containers/SparseArray.h"
#include "Containers/ContainerAllocationPolicies.h"

class FSetElementId
{
public:
    FORCEINLINE FSetElementId()
        : Index(-1)
    {
    }

    FORCEINLINE FSetElementId(int32 InIndex)
        : Index(InIndex)
    {
    }

    FORCEINLINE bool IsValidId() const
    {
        return Index != -1;
    }

    FORCEINLINE operator int32() const
    {
        return Index;
    }

private:
    int32 Index;
};

template<typename ElementType>
struct DefaultKeyFuncs
{
    typedef const ElementType& KeyInitType;
    typedef const ElementType& ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element;
    }

    static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
    {
        return A == B;
    }

    static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
    {
        return GetTypeHash(Key);
    }
};

template<typename InElementType>
class TSetElement
{
public:
    InElementType Value;
    mutable FSetElementId HashNextId;
    mutable int32 HashIndex;
};

template<typename InElementType, typename KeyFuncs = DefaultKeyFuncs<InElementType>>
class TSet
{
public:
    typedef InElementType ElementType;
    typedef typename KeyFuncs::KeyInitType KeyInitType;

    class TConstIterator
    {
    public:
        FORCEINLINE TConstIterator(const TSet& InSet, int32 InIndex)
            : Set(InSet)
            , Index(InIndex)
        {
            Skip();
        }

        FORCEINLINE TConstIterator& operator++()
        {
            ++Index;
            Skip();

            return *this;
        }

        FORCEINLINE const ElementType& operator*() const
        {
            return Set.Elements[Index].Value;
        }

        FORCEINLINE bool operator!=(const TConstIterator& Other) const
        {
            return Index != Other.Index;
        }

    private:
        FORCEINLINE void Skip()
        {
            while (Index < Set.Elements.GetMaxIndex() && !Set.Elements.IsAllocated(Index))
                ++Index;
        }

        const TSet& Set;
        int32 Index;
    };

    FORCEINLINE TSet()
        : HashSize(0)
    {
    }

    FORCEINLINE int32 Num() const
    {
        return Elements.Num();
    }

    FORCEINLINE ElementType& operator[](FSetElementId Id)
    {
        return Elements[Id].Value;
    }

    FSetElementId Add(const InElementType& InElement)
    {
        FSparseArrayAllocationInfo ElementAllocation = Elements.AddUninitialized();
        TSetElement<InElementType>& Element = *new(ElementAllocation) TSetElement<InElementType>{ InElement };
        FSetElementId ElementId(ElementAllocation.Index);

        if (Elements.Num() != 1)
        {
            FSetElementId ExistingId = FindId(KeyFuncs::GetSetKey(Element.Value));

            if (ExistingId.IsValidId())
            {
                Elements[ExistingId].Value.~InElementType();
                memmove((void*)&Elements[ExistingId].Value, (void*)&Element.Value, sizeof(InElementType));
                Elements.RemoveAtUninitialized(ElementId);

                return ExistingId;
            }
        }

        if (!ConditionalRehash(Elements.Num()))
        {
            HashElement(ElementId, Element);
        }

        return ElementId;
    }

    FORCEINLINE TConstIterator begin() const
    {
        return TConstIterator(*this, 0);
    }

    FORCEINLINE TConstIterator end() const
    {
        return TConstIterator(*this, Elements.GetMaxIndex());
    }

    FSetElementId FindId(KeyInitType Key) const
    {
        if (Elements.Num())
        {
            for (FSetElementId ElementId = GetTypedHash(KeyFuncs::GetKeyHash(Key)); ElementId.IsValidId(); ElementId = Elements[ElementId].HashNextId)
            {
                if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Elements[ElementId].Value), Key))
                {
                    return ElementId;
                }
            }
        }

        return FSetElementId();
    }

    FORCEINLINE ElementType* Find(KeyInitType Key)
    {
        FSetElementId ElementId = FindId(Key);

        return ElementId.IsValidId() ? &Elements[ElementId].Value : nullptr;
    }

    FORCEINLINE const ElementType* Find(KeyInitType Key) const
    {
        FSetElementId ElementId = FindId(Key);

        return ElementId.IsValidId() ? &Elements[ElementId].Value : nullptr;
    }

    FORCEINLINE bool Contains(KeyInitType Key) const
    {
        return FindId(Key).IsValidId();
    }

    void Remove(FSetElementId ElementId)
    {
        if (Elements.Num())
        {
            const auto& ElementBeingRemoved = Elements[ElementId];

            for (FSetElementId* NextElementId = &GetTypedHash(ElementBeingRemoved.HashIndex); NextElementId->IsValidId(); NextElementId = &Elements[*NextElementId].HashNextId)
            {
                if (*NextElementId == ElementId)
                {
                    *NextElementId = ElementBeingRemoved.HashNextId;
                    break;
                }
            }
        }

        Elements.RemoveAt(ElementId);
    }

    int32 Remove(KeyInitType Key)
    {
        int32 NumRemovedElements = 0;

        if (Elements.Num())
        {
            FSetElementId* NextElementId = &GetTypedHash(KeyFuncs::GetKeyHash(Key));

            while (NextElementId->IsValidId())
            {
                auto& Element = Elements[*NextElementId];

                if (KeyFuncs::Matches(KeyFuncs::GetSetKey(Element.Value), Key))
                {
                    Remove(*NextElementId);
                    NumRemovedElements++;
                    break;
                }
                else
                {
                    NextElementId = &Element.HashNextId;
                }
            }
        }

        return NumRemovedElements;
    }

    void Reset()
    {
        Elements.Reset();

        for (int32 HashIndex = 0, LocalHashSize = HashSize; HashIndex < LocalHashSize; ++HashIndex)
        {
            GetTypedHash(HashIndex) = FSetElementId();
        }
    }

private:
    FORCEINLINE FSetElementId& GetTypedHash(int32 HashIndex) const
    {
        return Hash.GetAllocation()[HashIndex & (HashSize - 1)];
    }

    static FORCEINLINE uint32 GetNumberOfHashBuckets(uint32 NumHashedElements)
    {
        if (NumHashedElements >= 4)
        {
            return FMath::RoundUpToPowerOfTwo(NumHashedElements / 2 + 8);
        }

        return 1;
    }

    FORCEINLINE void HashElement(FSetElementId ElementId, const TSetElement<InElementType>& Element) const
    {
        Element.HashIndex = KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(Element.Value)) & (HashSize - 1);
        Element.HashNextId = GetTypedHash(Element.HashIndex);
        GetTypedHash(Element.HashIndex) = ElementId;
    }

    FORCEINLINE bool ConditionalRehash(int32 NumHashedElements, bool bAllowShrinking = false) const
    {
        const int32 DesiredHashSize = GetNumberOfHashBuckets(NumHashedElements);

        if (NumHashedElements > 0 && (!HashSize || HashSize < DesiredHashSize || (HashSize > DesiredHashSize && bAllowShrinking)))
        {
            HashSize = DesiredHashSize;
            Rehash();

            return true;
        }

        return false;
    }

    void Rehash() const
    {
        Hash.ResizeAllocation(0, 0, sizeof(FSetElementId));

        int32 LocalHashSize = HashSize;

        if (LocalHashSize)
        {
            Hash.ResizeAllocation(0, LocalHashSize, sizeof(FSetElementId));

            for (int32 HashIndex = 0; HashIndex < LocalHashSize; ++HashIndex)
            {
                GetTypedHash(HashIndex) = FSetElementId();
            }

            for (int32 Index = 0; Index < Elements.GetMaxIndex(); Index++)
            {
                if (Elements.IsAllocated(Index))
                {
                    HashElement(FSetElementId(Index), Elements[Index]);
                }
            }
        }
    }

    TSparseArray<TSetElement<InElementType>> Elements;
    mutable TInlineAllocator<1>::ForElementType<FSetElementId> Hash;
    mutable int32 HashSize;
};
