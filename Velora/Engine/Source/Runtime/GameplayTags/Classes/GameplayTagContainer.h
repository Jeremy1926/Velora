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
};

struct FGameplayTagContainer
{
    FORCEINLINE bool HasTag(const FGameplayTag& TagToCheck) const
    {
        if (!TagToCheck.IsValid())
            return false;

        return GameplayTags.Contains(TagToCheck) || ParentTags.Contains(TagToCheck);
    }

    FORCEINLINE const TArray<FGameplayTag>& GetGameplayTagArray() const
    {
        return GameplayTags;
    }

    void AppendTags(FGameplayTagContainer const& Other);

    void AddTag(const FGameplayTag& TagToAdd);

    FString ToStringSimple(bool bQuoted = false) const;

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
