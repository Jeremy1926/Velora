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
        auto EquipWeaponDefinition = std::format("call {:#x}", (uintptr_t)Scanner::Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition")).Get(TEXT("AFortPawn::EquipWeaponDefinition")));

        return Anchor::Slot(TEXT("UFortWorldItemDefinition::ServerExecute"))
            .Virtual(TEXT("/Script/FortniteGame.Default__FortWeaponItemDefinition"), [&](const Anchor::FCode& Entry) { return Entry.Start().Has(EquipWeaponDefinition); })
            .Get();
    }();

    return Scanner::Virtual<bool (*)(const UFortWorldItemDefinition*, UFortWorldItem*, AFortPlayerController*)>(this, ServerExecute)(this, Item, PlayerController);
}

bool UFortWorldItemDefinition::SupportsQuickbarFocus() const
{
    static auto bSupportsQuickbarFocus = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bSupportsQuickbarFocus")));

    return bSupportsQuickbarFocus->GetPropertyValue_InContainer(this);
}

bool UFortWorldItemDefinition::CanBeDropped() const
{
    static auto bCanBeDropped = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bCanBeDropped")));

    return bCanBeDropped->GetPropertyValue_InContainer(this);
}

bool UFortWorldItemDefinition::ShouldDropOnDeath() const
{
    static auto bDropOnDeath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDropOnDeath")));

    return bDropOnDeath->GetPropertyValue_InContainer(this);
}

float UFortWorldItemDefinition::GetMaxDurability(int32 ItemLevel) const
{
    static auto GetMaxDurability = Anchor::Slot(TEXT("UFortWorldItemDefinition::GetMaxDurability"))
        .Exec(TEXT("/Script/FortniteGame.FortWorldItemDefinition:GetMaxDurability"))
        .Find("call [*+0x*]")
        .Get();

    return Scanner::Virtual<float (*)(const UFortWorldItemDefinition*, int32)>(this, GetMaxDurability)(this, ItemLevel);
}

const FDataTableCategoryHandle& UFortWorldItemDefinition::GetLootLevelData() const
{
    static auto LootLevelData = GetClass()->FindPropertyByName(FName(TEXT("LootLevelData")));

    return *LootLevelData->ContainerPtrToValuePtr<FDataTableCategoryHandle>(this);
}
