#include "pch.h"
#include "GameDataCosmetics.h"
#include "FortAssetManager.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"

UGameDataCosmetics* UGameDataCosmetics::Get()
{
    return UFortAssetManager::Get().GetGameDataCosmetics();
}

const TAssetPtr<UClass>& UGameDataCosmetics::GetEmoteGameplayAbility() const
{
    static auto EmoteGameplayAbility = GetClass()->FindPropertyByName(FName(TEXT("EmoteGameplayAbility")));

    return *EmoteGameplayAbility->ContainerPtrToValuePtr<TAssetPtr<UClass>>(this);
}

const TAssetPtr<UClass>* UGameDataCosmetics::GetSprayGameplayAbility() const
{
    static auto SprayGameplayAbility = GetClass()->FindPropertyByName(FName(TEXT("SprayGameplayAbility")));

    return SprayGameplayAbility ? SprayGameplayAbility->ContainerPtrToValuePtr<TAssetPtr<UClass>>(this) : nullptr;
}
