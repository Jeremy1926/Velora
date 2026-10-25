#pragma once

#include "Building/BuildingActor.h"

class AFortPawn;
class AFortPlayerController;
class AFortPlayerStateZone;
enum class EFortResourceType : uint8;

enum class EBuildingReplacementType : uint8
{
    BRT_None,
    BRT_Edited,
    BRT_Conversion,
    BRT_MAX
};

struct FBuildingSMActorAttemptSpawnResourcesParams
{
    AFortPawn* InstigatorPawn;
    FGameplayTagContainer Tags;
    float ActualDamageDealt;
    bool bJustHitWeakspot;
};

class ABuildingSMActor : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void SetMirrored(bool bIsMirrored);

    ABuildingSMActor* ReplaceBuildingActor(EBuildingReplacementType ReplacementType, UClass* NewBuildingClass, int32 BuildingLevel, int32 RotationIterations, bool bMirrored, AFortPlayerController* PlayerController);

    bool CanBePlayerEdited(AFortPlayerController* PlayerController);

    AFortPlayerStateZone* GetEditingPlayer() const;

    void SetEditingPlayer(AFortPlayerStateZone* NewEditingPlayer);

    bool IsBeingEdited() const;

    bool NeedsRepair() const;

    void RepairBuilding(AFortPlayerController* RepairingController, int32 ResourcesSpent);

    UObject* GetEditModePatternData() const;

    void AttemptSpawnResources(AFortPawn* InstigatorPawn, float Damage, bool bJustHitWeakspot);

    void AttemptSpawnResources(const FBuildingSMActorAttemptSpawnResourcesParams& Params);

    float DetermineMaxResourcesToSpawn(bool bRandomize);

    EFortResourceType GetResourceType() const;

    int32& GetMaxResourcesToSpawn();

    float& GetUndistributedResources();

    FName GetDestructionLootTierKey() const;
};
