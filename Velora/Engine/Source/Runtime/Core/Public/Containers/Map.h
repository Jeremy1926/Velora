#pragma once

#include "HAL/Platform.h"
#include "Containers/Set.h"

template<typename KeyType, typename ValueType>
struct TPair
{
    KeyType Key;
    ValueType Value;
};

template<typename KeyType, typename ValueType>
struct TDefaultMapKeyFuncs
{
    typedef const KeyType& KeyInitType;
    typedef const TPair<KeyType, ValueType>& ElementInitType;

    static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key;
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

template<typename KeyType, typename ValueType, typename KeyFuncs = TDefaultMapKeyFuncs<KeyType, ValueType>>
class TMap
{
public:
    FORCEINLINE int32 Num() const
    {
        return Pairs.Num();
    }

    FORCEINLINE ValueType& Add(const KeyType& InKey, const ValueType& InValue)
    {
        const FSetElementId PairId = Pairs.Add(TPair<KeyType, ValueType>{ InKey, InValue });

        return Pairs[PairId].Value;
    }

    FORCEINLINE ValueType* Find(const KeyType& Key)
    {
        auto Pair = Pairs.Find(Key);

        return Pair ? &Pair->Value : nullptr;
    }

    FORCEINLINE const ValueType* Find(const KeyType& Key) const
    {
        auto Pair = Pairs.Find(Key);

        return Pair ? &Pair->Value : nullptr;
    }

    FORCEINLINE ValueType& FindOrAdd(const KeyType& Key)
    {
        if (auto Pair = Pairs.Find(Key))
            return Pair->Value;

        return Add(Key, ValueType());
    }

    FORCEINLINE bool Contains(const KeyType& Key) const
    {
        return Pairs.Contains(Key);
    }

    FORCEINLINE int32 Remove(const KeyType& InKey)
    {
        return Pairs.Remove(InKey);
    }

    FORCEINLINE ValueType FindRef(const KeyType& Key) const
    {
        if (const auto Pair = Pairs.Find(Key))
        {
            return Pair->Value;
        }

        return ValueType();
    }

    FORCEINLINE ValueType& FindChecked(const KeyType& Key)
    {
        auto Pair = Pairs.Find(Key);

        return Pair->Value;
    }

    FORCEINLINE void Reset()
    {
        Pairs.Reset();
    }

    FORCEINLINE auto begin() const
    {
        return Pairs.begin();
    }

    FORCEINLINE auto end() const
    {
        return Pairs.end();
    }

private:
    TSet<TPair<KeyType, ValueType>, KeyFuncs> Pairs;
};
