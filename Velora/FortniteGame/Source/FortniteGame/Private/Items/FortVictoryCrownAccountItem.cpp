#include "pch.h"
#include "Items/FortVictoryCrownAccountItem.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortVictoryCrownAccountItem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVictoryCrownAccountItem"));

    return Class;
}

const FVictoryCrownAccountData& UFortVictoryCrownAccountItem::GetVictoryCrownAccountData() const
{
    static auto VictoryCrownAccountData = StaticClass()->FindPropertyByName(FName(TEXT("victory_crown_account_data")));

    return *VictoryCrownAccountData->ContainerPtrToValuePtr<FVictoryCrownAccountData>(this);
}
