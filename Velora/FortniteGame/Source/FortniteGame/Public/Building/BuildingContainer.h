#pragma once

#include "Building/BuildingAutoNav.h"
#include "Items/FortItemDefinition.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class AFortPawn;

struct FFortSearchBounceData
{
    FVector BounceNormal;
    uint32 SearchAnimationCount;
};

class ABuildingContainer : public ABuildingAutoNav
{
public:
    typedef ABuildingAutoNav Super;

    static UClass* StaticClass();

    void BeginPlay();

    bool IsAlreadySearched() const;

    void BounceContainer();

    bool SpawnLoot(AFortPawn* Searcher);

    void SetContainerSearched(AFortPawn* Searcher);

    FName GetSearchLootTierGroup() const;

    FName& GetContainerLootTierKey();

    FVector GetLootSpawnLocation() const;

    EFortRarity& GetHighestRarity();

    bool GetDestroyContainerOnSearch() const;

    bool GetStartAlreadySearched_Athena() const;

    float GetLootedWeaponsDurabilityModifier() const;

    FFortSearchBounceData& GetSearchBounceData();
};
