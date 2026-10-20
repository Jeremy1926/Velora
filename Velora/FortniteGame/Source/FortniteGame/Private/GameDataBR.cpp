#include "pch.h"
#include "GameDataBR.h"
#include "FortAssetManager.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "Scanner.h"

UGameDataBR* UGameDataBR::Get()
{
    return UFortAssetManager::Get().GetGameDataBR();
}

UFortAbilitySet* UGameDataBR::GetPlayerAbilitySetBR() const
{
    static auto GetPlayerAbilitySetBR = Scanner::Function<UFortAbilitySet* (*)(const UGameDataBR*)>(Scanner::String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!")).Start().Find("call 0x*; mov rcx, rax; call 0x*").Call(2).Get(TEXT("UGameDataBR::GetPlayerAbilitySetBR")));

    return GetPlayerAbilitySetBR(this);
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
