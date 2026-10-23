#include "pch.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Scanner.h"

FGameplayTag FGameplayTag::RequestGameplayTag(FName TagName, bool ErrorIfNotFound)
{
    return UGameplayTagsManager::Get().RequestGameplayTag(TagName, ErrorIfNotFound);
}

FString FGameplayTagContainer::ToStringSimple(bool bQuoted) const
{
    FString RetString;

    for (int i = 0; i < GameplayTags.Num(); ++i)
    {
        if (bQuoted)
        {
            RetString += TEXT("\"");
        }

        RetString += GameplayTags[i].ToString();

        if (bQuoted)
        {
            RetString += TEXT("\"");
        }

        if (i < GameplayTags.Num() - 1)
        {
            RetString += TEXT(", ");
        }
    }

    return RetString;
}

FString FGameplayTagContainer::ToString() const
{
    static auto ToString = Anchor::Function<FString* (*)(const FGameplayTagContainer*, FString*)>(TEXT("FGameplayTagContainer::ToString"))
        .String(TEXT("ability tag: %s"))
        .Last("call 0x*")
        .Target()
        .Get();

    FString Result;
    ToString(this, &Result);

    return Result;
}

void FGameplayTagContainer::AppendTags(FGameplayTagContainer const& Other)
{
    for (const FGameplayTag& OtherTag : Other.GameplayTags)
    {
        if (!GameplayTags.Contains(OtherTag))
            GameplayTags.Add(OtherTag);
    }

    for (const FGameplayTag& OtherTag : Other.ParentTags)
    {
        if (!ParentTags.Contains(OtherTag))
            ParentTags.Add(OtherTag);
    }
}

void FGameplayTagContainer::AddTag(const FGameplayTag& TagToAdd)
{
    static auto AddTag = Anchor::Function<void (*)(FGameplayTagContainer*, const FGameplayTag&)>(TEXT("FGameplayTagContainer::AddTag"))
        .Exec(TEXT("/Script/GameplayTags.BlueprintGameplayTagLibrary:AddGameplayTag"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/GameplayTags.BlueprintGameplayTagLibrary:AddGameplayTag"))
        .Call()
        .Get();

    AddTag(this, TagToAdd);
}

FGameplayTagQuery::FGameplayTagQuery()
    : TokenStreamVersion(EGameplayTagQueryStreamVersion::LatestVersion)
{
}

void FGameplayTagQuery::Build(FGameplayTagQueryExpression& RootQueryExpr, FString InUserDescription)
{
    TokenStreamVersion = EGameplayTagQueryStreamVersion::LatestVersion;
    UserDescription = InUserDescription;

    QueryTokenStream.Reset(128);
    TagDictionary.Reset();

    QueryTokenStream.Add(EGameplayTagQueryStreamVersion::LatestVersion);

    QueryTokenStream.Add(1);
    RootQueryExpr.EmitTokens(QueryTokenStream, TagDictionary);
}

FGameplayTagQuery FGameplayTagQuery::BuildQuery(FGameplayTagQueryExpression& RootQueryExpr, FString InDescription)
{
    FGameplayTagQuery Q;
    Q.Build(RootQueryExpr, InDescription);
    return Q;
}

bool FGameplayTagQuery::Matches(const FGameplayTagContainer& Tags) const
{
    static auto QueryMatches = Anchor::Optional<bool (*)(const FGameplayTagQuery*, const FGameplayTagContainer&)>(TEXT("FGameplayTagQuery::Matches"))
        .Native(TEXT("/Script/GameplayTags.BlueprintGameplayTagLibrary:DoesContainerMatchTagQuery"))
        .Is("mov rax, rsp; mov [rax+0x*], rbx; push rdi; sub rsp, 0x*; mov [rax-0x*], rcx*")
        .Get();

    if (QueryMatches)
        return QueryMatches(this, Tags);

    static auto DoesContainerMatchTagQuery = Anchor::Function<bool (*)(const FGameplayTagContainer&, const FGameplayTagQuery&)>(TEXT("UBlueprintGameplayTagLibrary::DoesContainerMatchTagQuery"))
        .Native(TEXT("/Script/GameplayTags.BlueprintGameplayTagLibrary:DoesContainerMatchTagQuery"))
        .Get();

    return DoesContainerMatchTagQuery(Tags, *this);
}

FGameplayTagQuery FGameplayTagQuery::MakeQuery_MatchTag(FGameplayTag const& InTag)
{
    return FGameplayTagQuery::BuildQuery
    (
        FGameplayTagQueryExpression()
        .AllTagsMatch()
        .AddTag(InTag)
    );
}

void FGameplayTagQueryExpression::EmitTokens(TArray<uint8>& TokenStream, TArray<FGameplayTag>& TagDictionary) const
{
    TokenStream.Add(ExprType);

    switch (ExprType)
    {
    case EGameplayTagQueryExprType::AnyTagsMatch:
    case EGameplayTagQueryExprType::AllTagsMatch:
    case EGameplayTagQueryExprType::NoTagsMatch:
    {
        uint8 NumTags = (uint8)TagSet.Num();
        TokenStream.Add(NumTags);

        for (auto Tag : TagSet)
        {
            int32 TagIdx = TagDictionary.AddUnique(Tag);
            TokenStream.Add((uint8)TagIdx);
        }
    }
    break;

    case EGameplayTagQueryExprType::AnyExprMatch:
    case EGameplayTagQueryExprType::AllExprMatch:
    case EGameplayTagQueryExprType::NoExprMatch:
    {
        uint8 NumExprs = (uint8)ExprSet.Num();
        TokenStream.Add(NumExprs);

        for (auto& E : ExprSet)
        {
            E.EmitTokens(TokenStream, TagDictionary);
        }
    }
    break;
    default:
        break;
    }
}
