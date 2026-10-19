#include "pch.h"
#include "Items/FortItem.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortItemDefinition* UFortItem::GetItemDefinitionBP() const
{
    static auto GetItemDefinitionBP = Scanner::Exec(TEXT("/Script/FortniteGame.FortItem:GetItemDefinitionBP")).Find("call [*]").Slot(TEXT("UFortItem::GetItemDefinitionBP"));

    return Scanner::Virtual<UFortItemDefinition* (*)(const UFortItem*)>(this, GetItemDefinitionBP)(this);
}

FGuid UFortItem::GetItemGuid() const
{
    static auto GetItemGuid = Scanner::Exec(TEXT("/Script/FortniteGame.FortItem:GetItemGuid")).Find("call [*]").Slot(TEXT("UFortItem::GetItemGuid"));

    FGuid Result;
    Scanner::Virtual<FGuid* (*)(const UFortItem*, FGuid*)>(this, GetItemGuid)(this, &Result);

    return Result;
}
