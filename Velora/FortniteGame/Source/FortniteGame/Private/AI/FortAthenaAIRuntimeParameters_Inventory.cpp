#include "pch.h"
#include "AI/FortAthenaAIRuntimeParameters_Inventory.h"
#include "GameplayTagContainer.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FAILootInfoRowEntry::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteAI.AILootInfoRowEntry"));

    return Struct;
}

const FGameplayTagQuery& FAILootInfoRowEntry::GetOptionalTagQuery() const
{
    static auto OptionalTagQuery = StaticStruct()->FindPropertyByName(FName(TEXT("OptionalTagQuery")));

    return *OptionalTagQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(this);
}

const FGameplayTagQuery& FAILootInfoRowEntry::GetOptionalPlaylistTagQuery() const
{
    static auto OptionalPlaylistTagQuery = StaticStruct()->FindPropertyByName(FName(TEXT("OptionalPlaylistTagQuery")));

    return *OptionalPlaylistTagQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(this);
}

bool FAILootInfoRowEntry::GetbShouldDropInventoryOnDeath() const
{
    static auto bShouldDropInventoryOnDeath = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bShouldDropInventoryOnDeath")));

    return bShouldDropInventoryOnDeath->GetPropertyValue_InContainer(this);
}

bool FAILootInfoRowEntry::GetbShouldDropLootOnDeath() const
{
    static auto bShouldDropLootOnDeath = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bShouldDropLootOnDeath")));

    return bShouldDropLootOnDeath->GetPropertyValue_InContainer(this);
}

bool FAILootInfoRowEntry::GetbShouldBotPickRandomLootTier() const
{
    static auto bShouldBotPickRandomLootTier = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bShouldBotPickRandomLootTier")));

    return bShouldBotPickRandomLootTier->GetPropertyValue_InContainer(this);
}

const TArray<FName>& FAILootInfoRowEntry::GetLootTiers() const
{
    static auto LootTiers = StaticStruct()->FindPropertyByName(FName(TEXT("LootTiers")));

    return *LootTiers->ContainerPtrToValuePtr<TArray<FName>>(this);
}

UScriptStruct* FFortAthenaAILootInfoDataTableRow::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteAI.FortAthenaAILootInfoDataTableRow"));

    return Struct;
}

int32 FFortAthenaAILootInfoDataTableRow::GetNumLootDroppingBehaviors() const
{
    static auto LootDroppingBehaviors = StaticStruct()->FindPropertyByName(FName(TEXT("LootDroppingBehaviors")));

    return LootDroppingBehaviors->ContainerPtrToValuePtr<TArray<uint8>>(this)->Num();
}

const FAILootInfoRowEntry& FFortAthenaAILootInfoDataTableRow::GetLootDroppingBehavior(int32 Index) const
{
    static auto LootDroppingBehaviors = (UArrayProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("LootDroppingBehaviors")));
    static auto ElementSize = LootDroppingBehaviors->GetInner()->GetSize();

    return *(const FAILootInfoRowEntry*)(LootDroppingBehaviors->ContainerPtrToValuePtr<TArray<uint8>>(this)->GetData() + Index * ElementSize);
}

UClass* UFortAthenaAIRuntimeParameters_Inventory::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIRuntimeParameters_Inventory"));

    return Class;
}

bool UFortAthenaAIRuntimeParameters_Inventory::GetbShouldDropCurrencyOnDeath() const
{
    static auto bShouldDropCurrencyOnDeath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bShouldDropCurrencyOnDeath")));

    return bShouldDropCurrencyOnDeath && bShouldDropCurrencyOnDeath->GetPropertyValue_InContainer(this);
}

FName UFortAthenaAIRuntimeParameters_Inventory::GetCurrencyPayoutRowName() const
{
    static auto CurrencyPayoutRowName = GetClass()->FindPropertyByName(FName(TEXT("CurrencyPayoutRowName")));

    return CurrencyPayoutRowName ? CurrencyPayoutRowName->GetPropertyValue_InContainer<FName>(this) : FName(NAME_None);
}

const FDataTableRowHandle& UFortAthenaAIRuntimeParameters_Inventory::GetLootInfo() const
{
    static auto LootInfo = GetClass()->FindPropertyByName(FName(TEXT("LootInfo")));

    return *LootInfo->ContainerPtrToValuePtr<FDataTableRowHandle>(this);
}
