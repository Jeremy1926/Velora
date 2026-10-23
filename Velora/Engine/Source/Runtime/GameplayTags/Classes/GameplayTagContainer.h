#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

struct FGameplayTag
{
    FGameplayTag()
        : TagName(NAME_None)
    {
    }

    static FGameplayTag RequestGameplayTag(FName TagName, bool ErrorIfNotFound = true);

    static FORCEINLINE int32 GetTypeSize()
    {
        return TGameLayout<FName>::Size();
    }

    FORCEINLINE bool operator==(FGameplayTag const& Other) const
    {
        return TagName == Other.TagName;
    }

    FORCEINLINE bool IsValid() const
    {
        return (TagName != NAME_None);
    }

    FORCEINLINE FName GetTagName() const
    {
        return TagName;
    }

    FORCEINLINE FString ToString() const
    {
        return TagName.ToString();
    }

protected:
    FName TagName;

private:
    friend struct TGameLayout<FGameplayTag>;

    explicit FGameplayTag(const FName& InTagName)
        : TagName(InTagName)
    {
    }
};

template<>
struct TGameLayout<FGameplayTag>
{
    using FStorage = FGameplayTag;

    static FORCEINLINE FGameplayTag Load(const void* Game)
    {
        return FGameplayTag(TGameLayout<FName>::Load(Game));
    }

    static FORCEINLINE void Store(void* Game, const FGameplayTag& Value)
    {
        TGameLayout<FName>::Store(Game, Value.TagName);
    }
};

template<>
class TArray<FGameplayTag, FDefaultAllocator> : public TRuntimeSizedArray<FGameplayTag>
{
public:
    class TConstIterator
    {
    public:
        FORCEINLINE TConstIterator(const uint8* InElement)
            : Element(InElement)
        {
        }

        FORCEINLINE const FGameplayTag operator*() const
        {
            return TGameLayout<FGameplayTag>::Load(Element);
        }

        FORCEINLINE TConstIterator& operator++()
        {
            Element += FGameplayTag::GetTypeSize();

            return *this;
        }

        FORCEINLINE bool operator!=(const TConstIterator& Other) const
        {
            return Element != Other.Element;
        }

    private:
        const uint8* Element;
    };

    FORCEINLINE TArray()
    {
    }

    TArray(const TArray& Other)
    {
        CopyToEmpty(Other, 0);
    }

    TArray(TArray&& Other)
    {
        MoveToEmpty(Other);
    }

    TArray& operator=(const TArray& Other)
    {
        if (this != &Other)
            CopyToEmpty(Other, ArrayMax);

        return *this;
    }

    TArray& operator=(TArray&& Other)
    {
        if (this != &Other)
            MoveToEmpty(Other);

        return *this;
    }

    FORCEINLINE const FGameplayTag operator[](int32 Index) const
    {
        return TGameLayout<FGameplayTag>::Load(&TRuntimeSizedArray::operator[](Index));
    }

    FORCEINLINE TConstIterator begin() const
    {
        return TConstIterator((const uint8*)AllocatorInstance.GetAllocation());
    }

    FORCEINLINE TConstIterator end() const
    {
        return TConstIterator((const uint8*)AllocatorInstance.GetAllocation() + ArrayNum * FGameplayTag::GetTypeSize());
    }

    int32 Add(const FGameplayTag& Item)
    {
        const int32 Index = AddUninitialized(1);
        TGameLayout<FGameplayTag>::Store(&TRuntimeSizedArray::operator[](Index), Item);

        return Index;
    }

    int32 AddUnique(const FGameplayTag& Item)
    {
        for (int32 Index = 0; Index < ArrayNum; Index++)
        {
            if ((*this)[Index] == Item)
                return Index;
        }

        return Add(Item);
    }

    bool Contains(const FGameplayTag& Item) const
    {
        for (const FGameplayTag& Element : *this)
        {
            if (Element == Item)
                return true;
        }

        return false;
    }

    void Reset()
    {
        ArrayNum = 0;
    }

private:
    void CopyToEmpty(const TArray& Other, int32 PrevMax)
    {
        const int32 NewMax = Other.ArrayNum ? AllocatorInstance.CalculateSlackReserve(Other.ArrayNum, FGameplayTag::GetTypeSize()) : 0;

        if (NewMax != PrevMax)
            AllocatorInstance.ResizeAllocation(0, NewMax, FGameplayTag::GetTypeSize());

        ArrayNum = Other.ArrayNum;
        ArrayMax = NewMax;

        if (ArrayNum)
            memcpy(AllocatorInstance.GetAllocation(), Other.AllocatorInstance.GetAllocation(), ArrayNum * FGameplayTag::GetTypeSize());
    }

    void MoveToEmpty(TArray& Other)
    {
        AllocatorInstance.MoveToEmpty(Other.AllocatorInstance);
        ArrayNum = Other.ArrayNum;
        ArrayMax = Other.ArrayMax;
        Other.ArrayNum = 0;
        Other.ArrayMax = 0;
    }
};

struct FGameplayTagContainer
{
    FORCEINLINE bool HasTag(const FGameplayTag& TagToCheck) const
    {
        if (!TagToCheck.IsValid())
            return false;

        return GameplayTags.Contains(TagToCheck) || ParentTags.Contains(TagToCheck);
    }

    FORCEINLINE bool HasAny(const FGameplayTagContainer& ContainerToCheck) const
    {
        if (ContainerToCheck.IsEmpty())
            return false;

        for (const FGameplayTag& OtherTag : ContainerToCheck.GameplayTags)
        {
            if (GameplayTags.Contains(OtherTag) || ParentTags.Contains(OtherTag))
                return true;
        }

        return false;
    }

    FORCEINLINE bool IsEmpty() const
    {
        return GameplayTags.Num() == 0;
    }

    FORCEINLINE const TArray<FGameplayTag>& GetGameplayTagArray() const
    {
        return GameplayTags;
    }

    void AppendTags(FGameplayTagContainer const& Other);

    void AddTag(const FGameplayTag& TagToAdd);

    FString ToStringSimple(bool bQuoted = false) const;

    FString ToString() const;

protected:
    TArray<FGameplayTag> GameplayTags;
    TArray<FGameplayTag> ParentTags;
};

namespace EGameplayTagQueryExprType
{
    enum Type
    {
        Undefined = 0,
        AnyTagsMatch,
        AllTagsMatch,
        NoTagsMatch,
        AnyExprMatch,
        AllExprMatch,
        NoExprMatch,
    };
}

namespace EGameplayTagQueryStreamVersion
{
    enum Type
    {
        InitialVersion = 0,

        VersionPlusOne,
        LatestVersion = VersionPlusOne - 1
    };
}

struct FGameplayTagQuery
{
    FGameplayTagQuery();

    void Build(struct FGameplayTagQueryExpression& RootQueryExpr, FString InUserDescription = FString());

    static FGameplayTagQuery BuildQuery(struct FGameplayTagQueryExpression& RootQueryExpr, FString InDescription = FString());

    static FGameplayTagQuery MakeQuery_MatchTag(FGameplayTag const& InTag);

    bool Matches(const FGameplayTagContainer& Tags) const;

    FORCEINLINE bool IsEmpty() const
    {
        return QueryTokenStream.Num() == 0;
    }

private:
    int32 TokenStreamVersion;
    TArray<FGameplayTag> TagDictionary;
    TArray<uint8> QueryTokenStream;
    FString UserDescription;
    FString AutoDescription;
};

struct FGameplayTagQueryExpression
{
    FGameplayTagQueryExpression& AllTagsMatch()
    {
        ExprType = EGameplayTagQueryExprType::AllTagsMatch;
        return *this;
    }

    FGameplayTagQueryExpression& AddTag(FGameplayTag Tag)
    {
        TagSet.Add(Tag);
        return *this;
    }

    void EmitTokens(TArray<uint8>& TokenStream, TArray<FGameplayTag>& TagDictionary) const;

    EGameplayTagQueryExprType::Type ExprType;
    TArray<struct FGameplayTagQueryExpression> ExprSet;
    TArray<FGameplayTag> TagSet;
};
