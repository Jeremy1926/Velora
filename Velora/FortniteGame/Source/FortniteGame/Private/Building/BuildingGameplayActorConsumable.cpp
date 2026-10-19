#include "pch.h"
#include "Building/BuildingGameplayActorConsumable.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

bool ABuildingGameplayActorConsumable::GetSpawnerCalculateRandomRotation() const
{
    static auto bSpawnerCalculateRandomRotation = (UBoolProperty*)((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingGameplayActorConsumable")))->FindPropertyByName(FName(TEXT("bSpawnerCalculateRandomRotation")));

    return bSpawnerCalculateRandomRotation->GetPropertyValue_InContainer(this);
}
