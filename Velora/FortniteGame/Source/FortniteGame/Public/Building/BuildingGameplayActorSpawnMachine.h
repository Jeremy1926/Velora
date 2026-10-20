#pragma once

#include "Building/BuildingActor.h"
#include "Containers/Array.h"
#include "GameFramework/OnlineReplStructs.h"
#include "UObject/NameTypes.h"

class AFortPlayerController;
class AFortPlayerPawn;
class UCapsuleComponent;
class UStaticMeshComponent;
struct FScalableFloat;

enum class ESpawnMachineState : uint8
{
    Default,
    WaitingForUse,
    Active,
    Complete,
    OnCooldown,
};

class ABuildingGameplayActorSpawnMachine : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void HandleOnResurrectTimerComplete();

    void OnResurrectionCompleted();

    void OnPlayerPawnResurrected(AFortPlayerPawn* PlayerPawn);

    bool RebootPlayersInAir() const;

    void KnockPlayerPawns(AFortPlayerPawn* IgnoreKnock);

    void KnockVehicles();

    void TriggerRespawnFX(AFortPlayerPawn* PhaseInPawn);

    void GiveWeaponsAndResources(AFortPlayerController* FortPC);

    void SkydiveIfAirborne(AFortPlayerPawn* Player);

    static void TickPendingResurrections();

    TArray<FUniqueNetIdRepl>& GetPlayerIdsForResurrection();

    uint8 GetSquadId() const;

    ESpawnMachineState GetSpawnMachineState() const;

    FScalableFloat& GetResurrectionNextPlayerDelay();

    UCapsuleComponent* GetBlockingVolume() const;

    UStaticMeshComponent* GetPadCollision() const;

    FName GetGrantedItemsLootTier() const;

    float GetHeightCheckForSkydive() const;

    float GetXYKnockAmountPlayer() const;

    float GetZKnockAmountPlayer() const;

    float GetXYKnockAmountVehicle() const;

    float GetZKnockAmountVehicle() const;
};
