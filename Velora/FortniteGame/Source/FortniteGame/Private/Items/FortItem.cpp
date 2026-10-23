#include "pch.h"
#include "Items/FortItem.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortItemDefinition* UFortItem::GetItemDefinitionBP() const
{
    static auto GetItemDefinitionBP = Anchor::VirtualFunction<UFortItemDefinition* (*)(const UFortItem*)>(TEXT("UFortItem::GetItemDefinitionBP"))
        .Exec(TEXT("/Script/FortniteGame.FortItem:GetItemDefinitionBP"))
        .Find("call [*]")
        .Get();

    return GetItemDefinitionBP(this);
}

FGuid UFortItem::GetItemGuid() const
{
    static auto GetItemGuid = Anchor::VirtualFunction<FGuid* (*)(const UFortItem*, FGuid*)>(TEXT("UFortItem::GetItemGuid"))
        .Exec(TEXT("/Script/FortniteGame.FortItem:GetItemGuid"))
        .Find("call [*]")
        .Get();

    FGuid Result;
    GetItemGuid(this, &Result);

    return Result;
}

FGuid UFortItem::GetTrackerGuid() const
{
    static auto bHasTrackerGuid = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItem:GetTrackerGuid")) != nullptr;

    if (!bHasTrackerGuid)
        return FGuid();

    static auto GetTrackerGuid = Anchor::VirtualFunction<FGuid* (*)(const UFortItem*, FGuid*)>(TEXT("UFortItem::GetTrackerGuid"))
        .Exec(TEXT("/Script/FortniteGame.FortItem:GetTrackerGuid"))
        .Find("call [*]")
        .Get();

    FGuid Result;
    GetTrackerGuid(this, &Result);

    return Result;
}

bool UFortItem::CanBeDropped() const
{
    static auto CanBeDropped = Anchor::VirtualFunction<bool (*)(const UFortItem*)>(TEXT("UFortItem::CanBeDropped"))
        .Exec(TEXT("/Script/FortniteGame.FortItem:CanBeDropped"))
        .Find("call [*]")
        .Get();

    return CanBeDropped(this);
}

int32 UFortItem::GetNumInStack() const
{
    static auto GetNumInStack = Anchor::VirtualFunction<int32 (*)(const UFortItem*)>(TEXT("UFortItem::GetNumInStack"))
        .Exec(TEXT("/Script/FortniteGame.FortItem:GetNumInStack"))
        .Find("call [*]")
        .Get();

    return GetNumInStack(this);
}
