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
