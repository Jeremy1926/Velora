#pragma once

#include "Building/BuildingActor.h"

class AFortPawn;
enum class EFortResourceType : uint8;

class ABuildingSMActor : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void AttemptSpawnResources(AFortPawn* InstigatorPawn, float Damage, bool bJustHitWeakspot);

    float DetermineMaxResourcesToSpawn(bool bRandomize);

    EFortResourceType GetResourceType() const;

    int32& GetMaxResourcesToSpawn();

    float& GetUndistributedResources();

    FName GetDestructionLootTierKey() const;
};
