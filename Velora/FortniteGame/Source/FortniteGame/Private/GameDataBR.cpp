#include "pch.h"
#include "GameDataBR.h"
#include "FortAssetManager.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPtr.h"
#include "Abilities/FortAbilitySet.h"
#include "Templates/Casts.h"
#include "Scanner.h"

UGameDataBR* UGameDataBR::Get()
{
    return UFortAssetManager::Get().GetGameDataBR();
}

UFortAbilitySet* UGameDataBR::GetPlayerAbilitySetBR() const
{
    static auto GetPlayerAbilitySetBR = Anchor::Optional<UFortAbilitySet* (*)(const UGameDataBR*)>(TEXT("UGameDataBR::GetPlayerAbilitySetBR"))
        .If(!Anchor::String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!")).Start().Find("call 0x*; mov rcx, rax; call 0x*; lea rcx, [rax+0x*]").Get())
        .String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!"))
        .Start()
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Call(2)
        .Get();

    if (GetPlayerAbilitySetBR)
        return GetPlayerAbilitySetBR(this);

    static auto PlayerAbilitySetBR = GetClass()->FindPropertyByName(FName(TEXT("PlayerAbilitySetBR")));

    return PlayerAbilitySetBR ? PlayerAbilitySetBR->ContainerPtrToValuePtr<TSoftObjectPtr<UFortAbilitySet>>(this)->LoadSynchronous() : nullptr;
}

UObject* UGameDataBR::GetFortWeaponPickupSpawnAmmoData() const
{
    static auto FortWeaponPickupSpawnAmmoData = GetClass()->FindPropertyByName(FName(TEXT("FortWeaponPickupSpawnAmmoData")));

    return FortWeaponPickupSpawnAmmoData ? FortWeaponPickupSpawnAmmoData->ContainerPtrToValuePtr<TSoftObjectPtr<UObject>>(this)->LoadSynchronous() : nullptr;
}

const TArray<TAssetPtr<UDataTable>>& UGameDataBR::GetLootTierDataTablesBR() const
{
    static auto LootTierDataTablesBR = GetClass()->FindPropertyByName(FName(TEXT("LootTierDataTablesBR")));

    return *LootTierDataTablesBR->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this);
}

const TArray<TAssetPtr<UDataTable>>& UGameDataBR::GetLootPackageDataTablesBR() const
{
    static auto LootPackageDataTablesBR = GetClass()->FindPropertyByName(FName(TEXT("LootPackageDataTablesBR")));

    return *LootPackageDataTablesBR->ContainerPtrToValuePtr<TArray<TAssetPtr<UDataTable>>>(this);
}
