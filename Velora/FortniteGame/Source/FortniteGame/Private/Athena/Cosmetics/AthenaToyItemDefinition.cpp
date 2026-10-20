#include "pch.h"
#include "Athena/Cosmetics/AthenaToyItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UAthenaToyItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaToyItemDefinition"));

    return Class;
}

const TAssetPtr<UClass>* UAthenaToyItemDefinition::GetToySpawnAbility() const
{
    static auto ToySpawnAbility = StaticClass()->FindPropertyByName(FName(TEXT("ToySpawnAbility")));

    return ToySpawnAbility ? ToySpawnAbility->ContainerPtrToValuePtr<TAssetPtr<UClass>>(this) : nullptr;
}
