#include "pch.h"
#include "FortGameData.h"
#include "FortAssetManager.h"
#include "Abilities/FortAbilitySet.h"
#include "UObject/AssetPtr.h"
#include "UObject/UnrealType.h"

UFortGameData* UFortGameData::Get()
{
    return UFortAssetManager::Get().GetGameData();
}

UFortAbilitySet* UFortGameData::GetGenericPlayerAbilitySet() const
{
    static auto GenericPlayerAbilitySet = GetClass()->FindPropertyByName(FName(TEXT("GenericPlayerAbilitySet")));

    return GenericPlayerAbilitySet->ContainerPtrToValuePtr<TAssetPtr<UFortAbilitySet>>(this)->LoadSynchronous();
}
