#include "pch.h"
#include "Items/FortWorldItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* UFortWorldItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldItemDefinition"));

    return Class;
}

bool UFortWorldItemDefinition::ServerExecute(UFortWorldItem* Item, AFortPlayerController* PlayerController) const
{
    static auto ServerExecute = []
    {
        auto EquipWeaponDefinition = std::format("call {:#x}", (uintptr_t)Scanner::Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition")).Get());
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWeaponItemDefinition"));
        auto Index = 0;

        while (!Scanner::At(VTable[Index]).Start().Has(EquipWeaponDefinition))
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const UFortWorldItemDefinition*, UFortWorldItem*, AFortPlayerController*))(VTable + ServerExecute))(this, Item, PlayerController);
}

float UFortWorldItemDefinition::GetMaxDurability(int32 ItemLevel) const
{
    static auto GetMaxDurability = FindFunctionChecked(FName(TEXT("GetMaxDurability")));

    struct
    {
        int32 ItemLevel;
        float ReturnValue;
    } Parms{ ItemLevel };

    const_cast<UFortWorldItemDefinition*>(this)->ProcessEvent(GetMaxDurability, &Parms);

    return Parms.ReturnValue;
}

const FDataTableCategoryHandle& UFortWorldItemDefinition::GetLootLevelData() const
{
    static auto LootLevelData = GetClass()->FindPropertyByName(FName(TEXT("LootLevelData")));

    return *LootLevelData->ContainerPtrToValuePtr<FDataTableCategoryHandle>(this);
}
