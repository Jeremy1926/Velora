#include "pch.h"
#include "Items/FortItem.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortItemDefinition* UFortItem::GetItemDefinitionBP() const
{
    static auto GetItemDefinitionBP = Scanner::Exec(TEXT("/Script/FortniteGame.FortItem:GetItemDefinitionBP")).Find("call [*]").Slot(TEXT("UFortItem::GetItemDefinitionBP"));

    auto VTable = *(uint8_t**)this;

    return (*(UFortItemDefinition* (**)(const UFortItem*))(VTable + GetItemDefinitionBP))(this);
}

FGuid UFortItem::GetItemGuid() const
{
    static auto GetItemGuid = Scanner::Exec(TEXT("/Script/FortniteGame.FortItem:GetItemGuid")).Find("call [*]").Slot(TEXT("UFortItem::GetItemGuid"));

    auto VTable = *(uint8_t**)this;
    FGuid Result;

    return *(*(FGuid* (**)(const UFortItem*, FGuid*))(VTable + GetItemGuid))(this, &Result);
}
