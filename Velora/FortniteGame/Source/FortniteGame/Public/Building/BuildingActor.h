#pragma once

#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "GameplayTagContainer.h"

class AController;
class AFortGameStateZone;
class UScriptStruct;
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
    static UScriptStruct* StaticStruct();

    FVector GetBuildLoc() const;

    FRotator GetBuildRot() const;

    bool GetMirrored() const;

    FBuildingClassData GetBuildingClassData(AFortGameStateZone* GameState) const;
};

class ABuildingActor : public AActor
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, ABuildingActor*, float>& GetNotifyBuildingNetCullDistChange();

    void BeginPlay();

    void SilentDie(bool bPropagateSilentDeath);

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
