#include "pch.h"
#include "FortGameData.h"
#include "FortAssetManager.h"
#include "FortGameMode.h"
#include "GameDataBR.h"
#include "GameDataCosmetics.h"
#include "Abilities/FortAbilitySet.h"
#include "UObject/AssetPtr.h"
#include "UObject/UnrealType.h"
#include "Items/FortLootTier.h"
#include "Items/FortLootPackage.h"
#include "FortniteGame.h"
#include "Logging/LogMacros.h"
#include "Items/FortResourceItemDefinition.h"
#include "Items/FortWeaponItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "Scanner.h"

UFortGameData* UFortGameData::Get()
{
    return UFortAssetManager::Get().GetGameData();
}

UFortAbilitySet* UFortGameData::GetGenericPlayerAbilitySet() const
{
    static auto GenericPlayerAbilitySet = GetClass()->FindPropertyByName(FName(TEXT("GenericPlayerAbilitySet")));

    return GenericPlayerAbilitySet->ContainerPtrToValuePtr<TAssetPtr<UFortAbilitySet>>(this)->LoadSynchronous();
}

UFortAbilitySet* UFortGameData::GetAthenaPlayerAbilitySet() const
{
    static auto AthenaPlayerAbilitySet = GetClass()->FindPropertyByName(FName(TEXT("AthenaPlayerAbilitySet")));
    static auto GenericPlayerAbilitySet = GetClass()->FindPropertyByName(FName(TEXT("GenericPlayerAbilitySet")));

    if (AthenaPlayerAbilitySet)
        return AthenaPlayerAbilitySet->ContainerPtrToValuePtr<TAssetPtr<UFortAbilitySet>>(this)->LoadSynchronous();

    return GenericPlayerAbilitySet ? GetGenericPlayerAbilitySet() : UGameDataBR::Get()->GetPlayerAbilitySetBR();
}

FText UFortGameData::GetBuildingResourceName(EFortResourceType ResourceType) const
{
    static auto GetBuildingResourceName = Anchor::Function<FText* (*)(const UFortGameData*, FText*, EFortResourceType)>(TEXT("UFortGameData::GetBuildingResourceName"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Call(2)
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Call(2)
        .Get();

    FText Result(NoInit);
    GetBuildingResourceName(this, &Result, ResourceType);

    return Result;
}

UFortWeaponItemDefinition* UFortGameData::GetEditToolItem() const
{
    static auto EditToolItem = GetClass()->FindPropertyByName(FName(TEXT("EditToolItem")));

    return EditToolItem->ContainerPtrToValuePtr<TAssetPtr<UFortWeaponItemDefinition>>(this)->LoadSynchronous();
}

const TAssetPtr<UClass>& UFortGameData::GetEmoteGameplayAbility() const
{
    static auto EmoteGameplayAbility = GetClass()->FindPropertyByName(FName(TEXT("EmoteGameplayAbility")));

    return EmoteGameplayAbility ? *EmoteGameplayAbility->ContainerPtrToValuePtr<TAssetPtr<UClass>>(this) : UGameDataCosmetics::Get()->GetEmoteGameplayAbility();
}

const TAssetPtr<UClass>* UFortGameData::GetSprayGameplayAbility() const
{
    static auto SprayGameplayAbility = GetClass()->FindPropertyByName(FName(TEXT("SprayGameplayAbility")));

    if (SprayGameplayAbility)
        return SprayGameplayAbility->ContainerPtrToValuePtr<TAssetPtr<UClass>>(this);

    auto GameDataCosmetics = UGameDataCosmetics::Get();

    return GameDataCosmetics ? GameDataCosmetics->GetSprayGameplayAbility() : nullptr;
}

const TArray<TAssetPtr<UClass>>* UFortGameData::GetAIDirectors() const
{
    static auto AIDirectors = GetClass()->FindPropertyByName(FName(TEXT("AIDirectors")));

    return AIDirectors ? AIDirectors->ContainerPtrToValuePtr<TArray<TAssetPtr<UClass>>>(this) : nullptr;
}

const TArray<TAssetPtr<UDataTable>>& UFortGameData::GetLootTierDataTables() const
{
    static auto LootTierDataTables = GetClass()->FindPropertyByName(FName(TEXT("LootTierDataTables")));

    return LootTierDataTables ? *LootTierDataTables->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this) : UGameDataBR::Get()->GetLootTierDataTablesBR();
}

const TArray<TAssetPtr<UDataTable>>& UFortGameData::GetLootPackageDataTables() const
{
    static auto LootPackageDataTables = GetClass()->FindPropertyByName(FName(TEXT("LootPackageDataTables")));

    return LootPackageDataTables ? *LootPackageDataTables->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this) : UGameDataBR::Get()->GetLootPackageDataTablesBR();
}

void UFortGameData::GetLootTierDataTables(TArray<UDataTable*>& OutLootTierDataTables) const
{
    if (auto GameMode = Cast<AFortGameMode>(static_cast<UGameEngine*>(GEngine)->GetGameInstance()->GetWorld()->GetAuthGameMode()))
        GameMode->GetLootTierDataTables(OutLootTierDataTables);
}

void UFortGameData::GetLootPackageDataTables(TArray<UDataTable*>& OutLootPackageDataTables) const
{
    if (auto GameMode = Cast<AFortGameMode>(static_cast<UGameEngine*>(GEngine)->GetGameInstance()->GetWorld()->GetAuthGameMode()))
        GameMode->GetLootPackageDataTables(OutLootPackageDataTables);
}

FFortLootTierData* UFortGameData::FindLootTierDataRow(FName TierGroupName, const FString& ContextString) const
{
    TArray<UDataTable*> LootTierDataTables;

    GetLootTierDataTables(LootTierDataTables);

    for (auto DataTable : LootTierDataTables)
    {
        if (auto LootTierData = DataTable->FindRow<FFortLootTierData>(TierGroupName, *ContextString, false))
            return LootTierData;
    }

    UE_LOG(LogFort, Warning, TEXT("UFortGameData::FindLootTierDataRow : TierGroupName %s with context %s not found!"), *TierGroupName.ToString(), *ContextString);

    return nullptr;
}

bool UFortGameData::HasLootPackageData() const
{
    TArray<UDataTable*> LootPackageDataTables;

    GetLootPackageDataTables(LootPackageDataTables);

    return LootPackageDataTables.Num() > 0;
}

UFortResourceItemDefinition* UFortGameData::GetResourceItemDefinition(EFortResourceType ResourceType) const
{
    static auto GetResourceItemDefinition = Anchor::Function<UFortResourceItemDefinition* (*)(const UFortGameData*, EFortResourceType)>(TEXT("UFortGameData::GetResourceItemDefinition"))
        .Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Call(2)
        .Or()
        .Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    return GetResourceItemDefinition(this, ResourceType);
}
