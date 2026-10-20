#pragma once

#include "GameFramework/Actor.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"

class AController;
class AFortGameStateZone;
struct FGameplayEffectContextHandle;

enum class EFortBuildingType : uint8
{
    Wall,
    Floor,
    Corner,
    Deco,
    Prop,
    Stairs,
    Roof,
    Pillar,
    SpawnedItem,
    Container,
    Trap,
    GenericCenterCellActor,
    None,
    EFortBuildingType_MAX
};

enum class EFortBuildingInitializationReason : uint8
{
    StaticallyPlaced,
    Spawned,
    Replaced,
    LoadedFromSave,
    DynamicBuilderPlaced,
    PlacementTool,
    TrapTool,
    None,
    EFortBuildingInitializationReason_MAX
};

struct FBuildingClassData
{
    UClass* BuildingClass;
    int32 PreviousBuildingLevel;
    int32 UpgradeLevel;
};

struct FCreateBuildingActorData
{
    uint32 BuildingClassHandle;
    FVector_NetQuantize10 BuildLoc;
    FRotator BuildRot;
    bool bMirrored;
    float SyncKey;
    FBuildingClassData BuildingClassData;

    FBuildingClassData GetBuildingClassData(AFortGameStateZone* GameState);
};

class ABuildingActor : public AActor
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    void SilentDie();

    void Die(FGameplayTag DamageTag, AController* EventInstigator, AActor* DamageCauser, const FGameplayEffectContextHandle* EffectContext);

    void InitializeBuildingActor(EFortBuildingInitializationReason InitializationReason, int32 InOwnerPersistentID, ABuildingActor* BuildingOwner, ABuildingActor* ReplacedBuilding, bool bFlag);

    void PostInitializeSpawnedBuildingActor(EFortBuildingInitializationReason InitializationReason);

    int32 GetCurrentBuildingLevel() const;

    void SetCurrentBuildingLevel(int32 NewLevel);

    bool GetDestroyOnPlayerBuildingPlacement() const;

    EFortBuildingType GetBuildingType() const;

    float GetMaxHealth() const;

    bool HasHealthLeft() const;

    const FGameplayTagContainer& GetStaticGameplayTags() const;

    bool GetDestroyed() const;
};
