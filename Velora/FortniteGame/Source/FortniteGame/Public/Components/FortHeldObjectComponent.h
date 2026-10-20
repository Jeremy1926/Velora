#pragma once

#include "Components/ActorComponent.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "UObject/AssetPtr.h"
#include "UObject/NameTypes.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class AController;
class AFortPlayerPawn;
class AFortWeapon;
class UFortProjectileMovementComponent;
class UFortWeaponItemDefinition;
class UFortWorldItem;
class UPrimitiveComponent;
struct FFrame;

enum class EHeldObjectState : uint8
{
    Unheld,
    Held,
    Thrown,
    Placed,
    Dropped,
};

class UFortHeldObjectComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void PickupHeldObject(AFortPlayerPawn* PlayerPawn);

    void DropHeldObject();

    void PlaceHeldObject();

    void ThrowHeldObject(const FVector& DetachLocation, const FRotator& ThrowDirection);

    void OnThrowComplete();

    void OnWeaponUnequipped();

    void HandleOwnerDestroyed(AActor* DestroyedActor);

    void HandleOwnerAsBuildingActorDestroyed(AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, const FVector& HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, const FVector& Momentum);

    void OnRep_OwningPawn();

    float GetGravityScale() const;

    AFortPlayerPawn* GetInstigator() const;

    float GetMaxSpeed() const;

    void SetupOwningPawn(AFortPlayerPawn* NewOwningPawn);

    void ClearOwningPawn();

    void DetachFromOwningPawn();

    void EquipBackingWeapon();

    void UnequipBackingWeapon();

    void execPickupHeldObject(FFrame& Stack, void* const Z_Param__Result);

    void execDropHeldObject(FFrame& Stack, void* const Z_Param__Result);

    void execPlaceHeldObject(FFrame& Stack, void* const Z_Param__Result);

    void execThrowHeldObject(FFrame& Stack, void* const Z_Param__Result);

    void execOnThrowComplete(FFrame& Stack, void* const Z_Param__Result);

    void execOnWeaponUnequipped(FFrame& Stack, void* const Z_Param__Result);

    void execHandleOwnerDestroyed(FFrame& Stack, void* const Z_Param__Result);

    void execHandleOwnerAsBuildingActorDestroyed(FFrame& Stack, void* const Z_Param__Result);

    TAssetPtr<UFortWeaponItemDefinition>& GetEquippedWeaponItemDefinition();

    FMulticastScriptDelegate& GetOnHeldObjectPickedUp();

    FMulticastScriptDelegate& GetOnHeldObjectPlaced();

    FMulticastScriptDelegate& GetOnHeldObjectDropped();

    FMulticastScriptDelegate& GetOnHeldObjectThrown();

    FMulticastScriptDelegate& GetOnHeldObjectDestroy();

    void SetDroppedFromWeaponSwap(bool bInDroppedFromWeaponSwap);

    EHeldObjectState& GetHeldObjectState();

    TWeakObjectPtr<AFortPlayerPawn>& GetOwningPawn();

    FRotator& GetObjectPickupRotation();

    TWeakObjectPtr<AFortPlayerPawn>& GetPreviousOwningPawn();

    TWeakObjectPtr<UFortWorldItem>& GetGrantedWeaponItem();

    TWeakObjectPtr<AFortWeapon>& GetGrantedWeapon();

    UFortProjectileMovementComponent* GetProjectileMovementComponent() const;
};
