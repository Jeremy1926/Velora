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
