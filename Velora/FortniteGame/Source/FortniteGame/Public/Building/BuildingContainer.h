#pragma once

#include "Building/BuildingAutoNav.h"
#include "Items/FortItemDefinition.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class AFortPawn;
class AFortPlayerPawn;
class UScriptStruct;

struct FFortSearchBounceData
{
    static UScriptStruct* StaticStruct();

    void SetBounceNormal(const FVector& InBounceNormal);

    uint32& GetSearchAnimationCount();

    void SetSearchingPawn(AFortPlayerPawn* InSearchingPawn);
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

    FName GetContainerLootTierKey();

    void SetContainerLootTierKey(FName InContainerLootTierKey);

    bool AthenaPostApplyPlaylistDataTables(bool bApplyRandomUpgrades, bool bReRollForLootExistence, bool bIgnorePotentialRandomUpgrades);

    int32 GetChosenRandomUpgrade() const;

    void SetReplicatedLootTier(int32 InReplicatedLootTier);

    void OnRep_LootTier();

    FVector GetLootSpawnLocation() const;

    EFortRarity& GetHighestRarity();

    bool ShouldDestroyContainerOnSearch() const;

    bool GetStartAlreadySearched_Athena() const;

    float GetLootedWeaponsDurabilityModifier() const;

    FFortSearchBounceData& GetSearchBounceData();
};
