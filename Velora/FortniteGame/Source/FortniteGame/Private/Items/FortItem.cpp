#include "pch.h"
#include "Items/FortItem.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortItemDefinition* UFortItem::GetItemDefinitionBP() const
{
    static auto GetItemDefinitionBP = Scanner::Exec(TEXT("/Script/FortniteGame.FortItem:GetItemDefinitionBP")).Find("call [*]").Disp();

    auto VTable = *(uint8_t**)this;

    return (*(UFortItemDefinition* (**)(const UFortItem*))(VTable + GetItemDefinitionBP))(this);
}

FGuid UFortItem::GetItemGuid() const
{
    static auto GetItemGuid = Scanner::Exec(TEXT("/Script/FortniteGame.FortItem:GetItemGuid")).Find("call [*]").Disp();

    auto VTable = *(uint8_t**)this;
    FGuid Result;

    return *(*(FGuid* (**)(const UFortItem*, FGuid*))(VTable + GetItemGuid))(this, &Result);
}
