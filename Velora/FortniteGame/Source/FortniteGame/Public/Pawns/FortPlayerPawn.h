#pragma once

#include "Pawns/FortPawn.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"
#include "UObject/ScriptDelegates.h"
#include "Delegates/Delegate.h"

class AActor;
class ABuildingGameplayActor;
class AController;
class AFortAthenaVehicle;
class AFortPickup;
class AFortPlayerPetRepState;
class UAthenaGliderItemDefinition;
class UFortMontageItemDefinitionBase;
class UFortWorldItem;
class UGameplayEffect;
class UScriptStruct;
class UStatManager;
struct FFrame;
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

struct FFortClientObservedStatArray
{
    static UScriptStruct* StaticStruct();

    void SetStatManager(UStatManager* InStatManager);

    UStatManager* GetMyStatManager() const;
};

class AFortPlayerPawn : public AFortPawn
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, AFortPlayerPawn*, AFortPlayerPetRepState*, AFortPlayerPetRepState*>& GetNotifyPetStateChange();

    static TMulticastDelegate<void, AFortPlayerPawn*, ABuildingGameplayActor*, ABuildingGameplayActor*>& GetNotifyBalloonStateChange();

    static TMulticastDelegate<void, AFortPlayerPawn*, AActor*, AActor*>& GetNotifyPossessedPropStateChange();

    static TMulticastDelegate<void, AFortPlayerPawn*, AActor*, AActor*>& GetVehicleStateChange();

    static TMulticastDelegate<void, AFortPlayerPawn*, bool>& GetNotifyPawnHighFrequencyUpdateChange();

    static FMulticastScriptDelegate* GetOnVehicleSpawnedEvent();

    FMulticastScriptDelegate& GetOnEnteredAircraft();

    AFortAthenaVehicle* GetVehicle() const;

    AActor* GetHeldObject() const;

    void SetHeldObject(AActor* InHeldObject);

    AFortAthenaVehicle* ServerOnExitVehicle(ETryExitVehicleBehavior ExitForceBehavior);

    void OnRep_InVehicle();

    FGuid* GetLastEquippedWeaponOrGadgetGUID();

    void SetLastEquippedWeaponOrGadget(UFortWorldItem* Item);

    void ServerEquipLastWeaponOrGadget_Implementation(bool bForce);

    void AddIgnoredActor(AActor* InActorToIgnore, const float InIgnoreDuration);

    const FVehiclePawnState& GetVehicleStateRep() const;

    const FVehiclePawnState& GetVehicleStateLocal() const;

    void BeginSkydiving(bool bFromBus);

    void OnBlueprintPlace();

    void ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    void ServerReviveFromDBNO_Implementation(AController* EventInstigator);

    void ForceReviveFromDBNO(AController* EventInstigator);

    void PlayGroupEmote(const UFortMontageItemDefinitionBase* EmoteAsset);

    void execPlayGroupEmote(FFrame& Stack, void* const Z_Param__Result);

    void UpdateGameStateModifiedGravity(float GravityMultiplier);

    void execUpdateGameStateModifiedGravity(FFrame& Stack, void* const Z_Param__Result);

    void SetClientObservedStatsFromAnotherPawn(AFortPlayerPawn* OtherPawn);

    void execSetClientObservedStatsFromAnotherPawn(FFrame& Stack, void* const Z_Param__Result);

    TArray<const UAthenaGliderItemDefinition*>& GetGliderOverrideStack();

    void PushGliderOverride(const UAthenaGliderItemDefinition* InGliderOverride);

    void execPushGliderOverride(FFrame& Stack, void* const Z_Param__Result);

    void PopGliderOverride(const UAthenaGliderItemDefinition* InGliderToRemove);

    void execPopGliderOverride(FFrame& Stack, void* const Z_Param__Result);

    void ServerSendZiplineState_Implementation(const FZiplinePawnState& InZiplineState);

    void HandleZiplineStateChanged();

    FZiplinePawnState& GetZiplineState();

    bool CanReviveFromDBNO(AController* EventInstigator);

    void UpdateDBNOInteractCollision();

    TSubclassOf<UGameplayEffect>& GetSelfReviveGameplayEffect();

    TSubclassOf<UGameplayEffect>& GetTeammateReviveGameplayEffect();

    FScalableFloat& GetSetByCallerReviveHealth();

    FGameplayTag GetEventReviveTag();

    FFortClientObservedStatArray& GetClientObservedStats();

    APawn* GetTetherPawn() const;
};
