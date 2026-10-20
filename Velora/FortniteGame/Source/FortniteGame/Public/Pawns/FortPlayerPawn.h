#pragma once

#include "Pawns/FortPawn.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"

class AActor;
class AController;
class AFortAthenaVehicle;
class AFortPickup;
class UGameplayEffect;
class UScriptStruct;
struct FGameplayTag;
struct FScalableFloat;

enum class ETryExitVehicleBehavior : uint8
{
    DoNotForce,
    ForceOnBlocking,
    ForceAlways,
};

struct FVehiclePawnState
{
    static UScriptStruct* StaticStruct();

    AActor* GetVehicle() const;

    uint8 GetSeatIndex() const;
};

struct FZiplinePawnState
{
    static UScriptStruct* StaticStruct();

    int32 GetAuthoritativeValue() const;
};

class AFortPlayerPawn : public AFortPawn
{
public:
    static UClass* StaticClass();

    AFortAthenaVehicle* GetVehicle() const;

    AActor* GetHeldObject() const;

    void SetHeldObject(AActor* InHeldObject);

    AFortAthenaVehicle* ServerOnExitVehicle(ETryExitVehicleBehavior ExitForceBehavior);

    void AddIgnoredActor(AActor* InActorToIgnore, const float InIgnoreDuration);

    const FVehiclePawnState& GetVehicleStateLocal() const;

    void BeginSkydiving(bool bFromBus);

    void OnBlueprintPlace();

    void ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    void ServerReviveFromDBNO_Implementation(AController* EventInstigator);

    void ServerSendZiplineState_Implementation(const FZiplinePawnState& InZiplineState);

    void HandleZiplineStateChanged();

    FZiplinePawnState& GetZiplineState();

    bool CanReviveFromDBNO(AController* EventInstigator);

    void UpdateDBNOInteractCollision();

    TSubclassOf<UGameplayEffect>& GetSelfReviveGameplayEffect();

    TSubclassOf<UGameplayEffect>& GetTeammateReviveGameplayEffect();

    FScalableFloat& GetSetByCallerReviveHealth();

    FGameplayTag& GetEventReviveTag();
};
