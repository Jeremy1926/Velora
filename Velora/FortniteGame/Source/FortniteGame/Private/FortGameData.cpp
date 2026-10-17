#include "pch.h"
#include "FortGameData.h"
#include "FortAssetManager.h"
#include "Abilities/FortAbilitySet.h"
#include "UObject/AssetPtr.h"
#include "UObject/UnrealType.h"
#include "Items/FortLootTier.h"
#include "Items/FortLootPackage.h"
#include "FortniteGame.h"
#include "Logging/LogMacros.h"
#include "Items/FortResourceItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "hde/hde64.h"

UFortGameData* UFortGameData::Get()
{
    return UFortAssetManager::Get().GetGameData();
}

UFortAbilitySet* UFortGameData::GetGenericPlayerAbilitySet() const
{
    static auto GenericPlayerAbilitySet = GetClass()->FindPropertyByName(FName(TEXT("GenericPlayerAbilitySet")));

    return GenericPlayerAbilitySet->ContainerPtrToValuePtr<TAssetPtr<UFortAbilitySet>>(this)->LoadSynchronous();
}

const TArray<TAssetPtr<UDataTable>>& UFortGameData::GetLootTierDataTables() const
{
    static auto LootTierDataTables = GetClass()->FindPropertyByName(FName(TEXT("LootTierDataTables")));

    return *LootTierDataTables->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this);
}

const TArray<TAssetPtr<UDataTable>>& UFortGameData::GetLootPackageDataTables() const
{
    static auto LootPackageDataTables = GetClass()->FindPropertyByName(FName(TEXT("LootPackageDataTables")));

    return *LootPackageDataTables->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this);
}

FFortLootTierData* UFortGameData::FindLootTierDataRow(FName TierGroupName, const FString& ContextString) const
{
    for (auto& LootTierDataTable : GetLootTierDataTables())
    {
        if (auto DataTable = LootTierDataTable.LoadSynchronous())
        {
            if (auto LootTierData = DataTable->FindRow<FFortLootTierData>(TierGroupName, *ContextString, false))
                return LootTierData;
        }
    }

    UE_LOG(LogFort, Warning, TEXT("UFortGameData::FindLootTierDataRow : TierGroupName %s with context %s not found!"), *TierGroupName.ToString(), *ContextString);

    return nullptr;
}

bool UFortGameData::HasLootPackageData() const
{
    for (auto& LootPackageDataTable : GetLootPackageDataTables())
    {
        if (LootPackageDataTable.LoadSynchronous())
            return true;
    }

    return false;
}

UFortResourceItemDefinition* UFortGameData::GetResourceItemDefinition(EFortResourceType ResourceType) const
{
    static auto GetResourceItemDefinition = []
    {
        const uint8_t* InventoryContextGetResourceItemDefinition = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition")))->GetNativeFunc()); !InventoryContextGetResourceItemDefinition;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                InventoryContextGetResourceItemDefinition = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        auto Calls = 0;

        for (auto Address = InventoryContextGetResourceItemDefinition;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if ((Instruction.opcode == 0xE8 || Instruction.opcode == 0xE9) && ++Calls == 2)
                return (UFortResourceItemDefinition* (*)(const UFortGameData*, EFortResourceType))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    return GetResourceItemDefinition(this, ResourceType);
}
