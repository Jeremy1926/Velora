#pragma once

#include "HAL/Platform.h"
#include "Containers/SparseArray.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Templates/TypeHash.h"
#include "Templates/UnrealTemplate.h"

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

struct FCompactSetLayout
{
    int32 Size;
    int32 Alignment;
};

class FCompactHashTableView
{
public:
    FORCEINLINE FCompactHashTableView(uint8* Memory, uint32 InNextIndexCount, uint32 InHashCount)
        : NextIndexData(Memory)
        , KeyStorage(Memory + ((SIZE_T)InNextIndexCount << GetTypeShift(InNextIndexCount)))
        , HashData(Memory + ((SIZE_T)InNextIndexCount << GetTypeShift(InNextIndexCount)) * 2)
        , NextIndexCount(InNextIndexCount)
        , HashCount(InHashCount)
    {
    }

    static FORCEINLINE uint32 GetTypeSize(uint32 IndexCount)
    {
        return 1 + int32(IndexCount > 0xff) + int32(IndexCount > 0xffff) * 2;
    }

    static FORCEINLINE uint32 GetTypeShift(uint32 IndexCount)
    {
        return 0 + int32(IndexCount > 0xff) + int32(IndexCount > 0xffff);
    }

    FORCEINLINE uint32 GetHashCount() const
    {
        return HashCount;
    }

    FORCEINLINE void Reset() const
    {
        FMemory::Memset(HashData, 0xff, (SIZE_T)HashCount << GetTypeShift(NextIndexCount));
    }

    FORCEINLINE uint32 GetFirst(uint32 Key) const
    {
        return Read(HashData, Key & (HashCount - 1));
    }

    FORCEINLINE uint32 GetNext(uint32 Index) const
    {
        return Read(NextIndexData, Index);
    }

    FORCEINLINE uint32 GetStoredKey(uint32 Index) const
    {
        return Read(KeyStorage, Index);
    }

    void Add(uint32 Index, uint32 Key) const
    {
        const uint32 HashIndex = Key & (HashCount - 1);

        Write(NextIndexData, Index, Read(HashData, HashIndex));
        Write(HashData, HashIndex, Index);
        Write(KeyStorage, Index, HashIndex);
    }

    uint32 Remove(uint32 Index, uint32 Key, uint32 LastIndex, uint32 OptLastKey) const
    {
        Write(KeyStorage, Index, Read(KeyStorage, LastIndex));

        uint8* LookupEntry = FindLookupEntry(Index, Key);
        WriteAt(LookupEntry, Read(NextIndexData, Index));

        if (Index != LastIndex)
        {
            WriteAt(FindLookupEntry(LastIndex, OptLastKey), Index);
            Write(NextIndexData, Index, Read(NextIndexData, LastIndex));
        }

        return ReadAt(LookupEntry);
    }

    FORCEINLINE void RemoveHashElement(uint32 Key) const
    {
        Write(HashData, Key & (HashCount - 1), INDEX_NONE);
    }

private:
    FORCEINLINE uint32 ReadAt(const uint8* Entry) const
    {
        switch (GetTypeSize(NextIndexCount))
        {
        case 1:
            return *Entry == 0xff ? (uint32)INDEX_NONE : *Entry;
        case 2:
            return *(const uint16*)Entry == 0xffff ? (uint32)INDEX_NONE : *(const uint16*)Entry;
        default:
            return *(const uint32*)Entry;
        }
    }

    FORCEINLINE void WriteAt(uint8* Entry, uint32 Value) const
    {
        switch (GetTypeSize(NextIndexCount))
        {
        case 1:
            *Entry = (uint8)Value;
            break;
        case 2:
            *(uint16*)Entry = (uint16)Value;
            break;
        default:
            *(uint32*)Entry = Value;
            break;
        }
    }

    FORCEINLINE uint32 Read(const uint8* Data, uint32 Index) const
    {
        return ReadAt(Data + ((SIZE_T)Index << GetTypeShift(NextIndexCount)));
    }

    FORCEINLINE void Write(uint8* Data, uint32 Index, uint32 Value) const
    {
        WriteAt(Data + ((SIZE_T)Index << GetTypeShift(NextIndexCount)), Value);
    }

    uint8* FindLookupEntry(uint32 Index, uint32 Key) const
    {
        for (uint8* Lookup = HashData + ((SIZE_T)(Key & (HashCount - 1)) << GetTypeShift(NextIndexCount)); ReadAt(Lookup) < NextIndexCount; Lookup = NextIndexData + ((SIZE_T)ReadAt(Lookup) << GetTypeShift(NextIndexCount)))
        {
            if (ReadAt(Lookup) == Index)
                return Lookup;
        }

        return nullptr;
    }

    uint8* NextIndexData;
    uint8* KeyStorage;
    uint8* HashData;
    uint32 NextIndexCount;
    uint32 HashCount;
};

class FCompactSetBase
{
public:
    static bool IsEnabled();

    int32 AllocatorCalculateSlackGrow(int32 NewMaxElements, const FCompactSetLayout& Layout) const;

    bool ResizeAllocationPreserveData(int32 NewMaxElements, const FCompactSetLayout& Layout, bool bPreserve = true);

    FORCEINLINE FCompactHashTableView GetHashTableView(const FCompactSetLayout& Layout) const
    {
        uint32* HashTable = (uint32*)(Elements + ((SIZE_T)(Layout.Size * MaxElements + 3) & ~(SIZE_T)3));

        return FCompactHashTableView((uint8*)(HashTable + 1), MaxElements, *HashTable);
    }

    uint8* Elements;
    int32 NumElements;
    int32 MaxElements;
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
            if (FCompactSetBase::IsEnabled())
                return Set.GetCompactData()[Index];

            return Set.Elements[Index].Value;
        }

        FORCEINLINE const ElementType* operator->() const
        {
            if (FCompactSetBase::IsEnabled())
                return &Set.GetCompactData()[Index];

            return &Set.Elements[Index].Value;
        }

        FORCEINLINE bool operator!=(const TConstIterator& Other) const
        {
            return Index != Other.Index;
        }

    private:
        FORCEINLINE void Skip()
        {
            if (FCompactSetBase::IsEnabled())
                return;

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

    FORCEINLINE ~TSet()
    {
        if (FCompactSetBase::IsEnabled())
        {
            auto& Compact = GetCompact();

            for (int32 Index = 0; Index < Compact.NumElements; Index++)
                GetCompactData()[Index].~InElementType();

            if (Compact.Elements)
                FMemory::Free(Compact.Elements);

            Compact.Elements = nullptr;
            Compact.NumElements = 0;
            Compact.MaxElements = 0;
        }
    }

    FORCEINLINE int32 Num() const
    {
        if (FCompactSetBase::IsEnabled())
            return GetCompact().NumElements;

        return Elements.Num();
    }

    FORCEINLINE ElementType& operator[](FSetElementId Id)
    {
        if (FCompactSetBase::IsEnabled())
            return GetCompactData()[Id];

        return Elements[Id].Value;
    }

    FSetElementId Add(const InElementType& InElement)
    {
        return Add(InElementType(InElement));
    }

    FSetElementId Add(InElementType&& InElement)
    {
        if (FCompactSetBase::IsEnabled())
            return CompactAdd(MoveTemp(InElement));

        FSparseArrayAllocationInfo ElementAllocation = Elements.AddUninitialized();
        TSetElement<InElementType>& Element = *new(ElementAllocation) TSetElement<InElementType>{ MoveTemp(InElement) };
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
        if (FCompactSetBase::IsEnabled())
            return TConstIterator(*this, GetCompact().NumElements);

        return TConstIterator(*this, Elements.GetMaxIndex());
    }

    FSetElementId FindId(KeyInitType Key) const
    {
        if (FCompactSetBase::IsEnabled())
            return CompactFindIndexByHash(KeyFuncs::GetKeyHash(Key), Key);

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

        if (FCompactSetBase::IsEnabled())
            return ElementId.IsValidId() ? &GetCompactData()[ElementId] : nullptr;

        return ElementId.IsValidId() ? &Elements[ElementId].Value : nullptr;
    }

    FORCEINLINE const ElementType* Find(KeyInitType Key) const
    {
        FSetElementId ElementId = FindId(Key);

        if (FCompactSetBase::IsEnabled())
            return ElementId.IsValidId() ? &GetCompactData()[ElementId] : nullptr;

        return ElementId.IsValidId() ? &Elements[ElementId].Value : nullptr;
    }

    FORCEINLINE bool Contains(KeyInitType Key) const
    {
        return FindId(Key).IsValidId();
    }

    void Remove(FSetElementId ElementId)
    {
        if (FCompactSetBase::IsEnabled())
        {
            auto HashTable = GetCompact().GetHashTableView(GetSetLayout());
            CompactRemoveByIndexAndHash(ElementId, HashTable.GetStoredKey(ElementId), HashTable);

            return;
        }

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
        if (FCompactSetBase::IsEnabled())
        {
            if (!GetCompact().NumElements)
                return 0;

            const uint32 KeyHash = KeyFuncs::GetKeyHash(Key);
            auto HashTable = GetCompact().GetHashTableView(GetSetLayout());

            for (uint32 ElementIndex = HashTable.GetFirst(KeyHash); ElementIndex != (uint32)INDEX_NONE; ElementIndex = HashTable.GetNext(ElementIndex))
            {
                if (KeyFuncs::Matches(KeyFuncs::GetSetKey(GetCompactData()[ElementIndex]), Key))
                {
                    CompactRemoveByIndexAndHash(ElementIndex, KeyHash, HashTable);

                    return 1;
                }
            }

            return 0;
        }

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
        if (FCompactSetBase::IsEnabled())
        {
            auto& Compact = GetCompact();

            if (Compact.NumElements > 0)
            {
                auto HashTable = Compact.GetHashTableView(GetSetLayout());

                if (Compact.NumElements < (int32)(HashTable.GetHashCount() / 4))
                {
                    for (int32 Index = 0; Index < Compact.NumElements; ++Index)
                        HashTable.RemoveHashElement(HashTable.GetStoredKey(Index));
                }
                else
                {
                    HashTable.Reset();
                }

                for (int32 Index = 0; Index < Compact.NumElements; Index++)
                    GetCompactData()[Index].~InElementType();

                Compact.NumElements = 0;
            }

            return;
        }

        Elements.Reset();

        for (int32 HashIndex = 0, LocalHashSize = HashSize; HashIndex < LocalHashSize; ++HashIndex)
        {
            GetTypedHash(HashIndex) = FSetElementId();
        }
    }

private:
    static FORCEINLINE FCompactSetLayout GetSetLayout()
    {
        return { (int32)sizeof(InElementType), alignof(InElementType) > 4 ? (int32)alignof(InElementType) : 4 };
    }

    FORCEINLINE FCompactSetBase& GetCompact() const
    {
        return *(FCompactSetBase*)this;
    }

    FORCEINLINE InElementType* GetCompactData() const
    {
        return (InElementType*)GetCompact().Elements;
    }

    int32 CompactFindIndexByHash(uint32 KeyHash, KeyInitType Key) const
    {
        if (GetCompact().NumElements == 0)
            return INDEX_NONE;

        auto HashTable = GetCompact().GetHashTableView(GetSetLayout());

        for (uint32 Index = HashTable.GetFirst(KeyHash); Index != (uint32)INDEX_NONE; Index = HashTable.GetNext(Index))
        {
            if (KeyFuncs::Matches(KeyFuncs::GetSetKey(GetCompactData()[Index]), Key))
                return Index;
        }

        return INDEX_NONE;
    }

    void CompactRehash()
    {
        auto& Compact = GetCompact();

        if (Compact.MaxElements > 0)
        {
            auto HashTable = Compact.GetHashTableView(GetSetLayout());
            HashTable.Reset();

            for (int32 Index = 0; Index < Compact.NumElements; ++Index)
                HashTable.Add(Index, KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(GetCompactData()[Index])));
        }
    }

    FSetElementId CompactAdd(InElementType&& InElement)
    {
        const uint32 KeyHash = KeyFuncs::GetKeyHash(KeyFuncs::GetSetKey(InElement));
        const int32 ExistingIndex = CompactFindIndexByHash(KeyHash, KeyFuncs::GetSetKey(InElement));

        if (ExistingIndex != INDEX_NONE)
        {
            GetCompactData()[ExistingIndex].~InElementType();
            new(&GetCompactData()[ExistingIndex]) InElementType(MoveTemp(InElement));

            return ExistingIndex;
        }

        auto& Compact = GetCompact();

        if (Compact.NumElements == Compact.MaxElements)
        {
            const int32 NewMaxElements = Compact.AllocatorCalculateSlackGrow(Compact.NumElements + 1, GetSetLayout());

            if ((uint32)NewMaxElements > (uint32)Compact.MaxElements && Compact.ResizeAllocationPreserveData(NewMaxElements, GetSetLayout()))
                CompactRehash();
        }

        Compact.GetHashTableView(GetSetLayout()).Add(Compact.NumElements, KeyHash);
        new(&GetCompactData()[Compact.NumElements]) InElementType(MoveTemp(InElement));

        return Compact.NumElements++;
    }

    void CompactRemoveByIndexAndHash(int32 ElementIndex, uint32 KeyHash, const FCompactHashTableView& HashTable)
    {
        auto& Compact = GetCompact();
        const int32 LastElementIndex = Compact.NumElements - 1;

        if (ElementIndex == LastElementIndex)
        {
            HashTable.Remove(ElementIndex, KeyHash, ElementIndex, KeyHash);
            GetCompactData()[LastElementIndex].~InElementType();
        }
        else
        {
            HashTable.Remove(ElementIndex, KeyHash, LastElementIndex, HashTable.GetStoredKey(LastElementIndex));
            GetCompactData()[ElementIndex].~InElementType();
            memmove((void*)&GetCompactData()[ElementIndex], (void*)&GetCompactData()[LastElementIndex], sizeof(InElementType));
        }

        --Compact.NumElements;
    }

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
