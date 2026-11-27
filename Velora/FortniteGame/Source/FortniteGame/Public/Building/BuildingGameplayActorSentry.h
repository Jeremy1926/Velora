#pragma once

#include "Building/BuildingGameplayActor.h"
#include "Containers/Array.h"
#include "Math/Rotator.h"
#include "Templates/UnrealTemplate.h"

class AController;
class USceneComponent;
struct FFrame;
struct FGameplayEffectContextHandle;
struct FHitResult;
struct FScalableFloat;
struct FTimerHandle;

enum class EBuildingGameplayActorSentry_State : uint8
{
    PassiveIdle = 0,
    ActiveIdle = 1,
    Tracking = 2,
    Aggro = 3,
    Dormant = 4,
    Deactivated = 5,
    ReturningToIdle = 6,
    LocatingDamager = 7,
    EBuildingGameplayActorSentry_MAX = 8
};

class ABuildingGameplayActorSentry : public ABuildingGameplayActor
{
public:
    static UClass* StaticClass();

    void SetSentryState(EBuildingGameplayActorSentry_State InSentryState);

    void execSetSentryState(FFrame& Stack, void* const Z_Param__Result);

    void SetActorToTrack(AActor* InActorToTrack);

    void execSetActorToTrack(FFrame& Stack, void* const Z_Param__Result);

    void BPOnTrackedActorSet(AActor* TrackedActorPrevious);

    void StartAggro();

    static void TickPendingAggros();

    void HandleDamaged(float Damage, const FGameplayTagContainer& DamageTags, const TGameValue<FVector>& Momentum, const FHitResult& HitInfo, AController* InstigatedBy, AActor* DamageCauser, const FGameplayEffectContextHandle& EffectContext);

    void SetLocatingDamageRotationDestination(const FRotator& InLocatingDamageRotationDestination);

    void ReturnToIdle();

    void IdleRestart();

    void SelectClosestIdleRotation();

    void SetFixedRotationGoal(const FRotator& InFixedRotationGoal);

    void SetFixedRotationGoalForReplication(const FRotator& InFixedRotationGoal);

    void OnRep_CurrentSentryState(EBuildingGameplayActorSentry_State PreviousSentryState);

    FTimerHandle* GetReturnToIdleTimerHandle();

    EBuildingGameplayActorSentry_State& GetCurrentSentryState();

    USceneComponent* GetCameraStaticMeshComp();

    bool GetbDoesNotRotate();

    bool GetbLocateDamagerIfHitWhileIdle();

    bool GetbGoDormantAfterPassiveIdle();

    bool GetbIdleRotatingRight();

    void SetbIdleRotationStopped(bool bInIdleRotationStopped);

    FRotator GetTargetRotation();

    FRotator GetTargetRotationRight();

    FRotator GetTargetRotationLeft();

    TArray<AActor*>& GetTargetActorsInRange();

    FGameplayTag GetReturnToIdleGameplayCueTag();

    AActor*& GetActorBeingTracked();

    FScalableFloat& GetTrackedToAggroDelay();

    float GetTrackedToAggroDelayMultiplier();

    FGameplayTag GetTrackedGameplayCueTag();

    FGameplayTag GetAggroedGameplayCueTag();
};
