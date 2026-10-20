#include "pch.h"
#include "Items/FortItem.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortItemDefinition* UFortItem::GetItemDefinitionBP() const
{
    static auto GetItemDefinitionBP = Anchor::Slot(TEXT("UFortItem::GetItemDefinitionBP"))
        .Exec(TEXT("/Script/FortniteGame.FortItem:GetItemDefinitionBP"))
        .Find("call [*]")
        .Get();

    return Scanner::Virtual<UFortItemDefinition* (*)(const UFortItem*)>(this, GetItemDefinitionBP)(this);
}

FGuid UFortItem::GetItemGuid() const
{
    static auto GetItemGuid = Anchor::Slot(TEXT("UFortItem::GetItemGuid"))
        .Exec(TEXT("/Script/FortniteGame.FortItem:GetItemGuid"))
        .Find("call [*]")
        .Get();

    FGuid Result;
    Scanner::Virtual<FGuid* (*)(const UFortItem*, FGuid*)>(this, GetItemGuid)(this, &Result);

    return Result;
}
