#include "pch.h"
#include "Components/FortHeldObjectComponent.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Pawns/FortPlayerPawn.h"
#include "Weapons/FortProjectileMovementComponent.h"
#include "Weapons/FortWeapon.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "WorldCollision.h"
#include "Math/Quat.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortHeldObjectComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortHeldObjectComponent"));

    return Class;
}

void UFortHeldObjectComponent::PickupHeldObject(AFortPlayerPawn* PlayerPawn)
{
    auto Owner = GetOwner();

    if (!PlayerPawn || !Owner || !Owner->HasAuthority() || GetOwningPawn())
        return;

    Owner->FlushNetDormancy();

    if (GetGrantedWeaponItem().IsValid())
        UnequipBackingWeapon();

    const FRotator OwnerRotation = Owner->GetActorRotation();
    const FRotator PawnRotation = PlayerPawn->GetActorRotation();

    GetObjectPickupRotation() = FRotator{ FRotator::NormalizeAxis(OwnerRotation.Pitch - PawnRotation.Pitch), FRotator::NormalizeAxis(OwnerRotation.Yaw - PawnRotation.Yaw), FRotator::NormalizeAxis(OwnerRotation.Roll - PawnRotation.Roll) };

    SetDroppedFromWeaponSwap(false);
    SetupOwningPawn(PlayerPawn);
    GetHeldObjectState() = EHeldObjectState::Held;

    EquipBackingWeapon();
    OnRep_OwningPawn(nullptr);

    GetOnHeldObjectPickedUp().ProcessMulticastDelegate<UObject>(nullptr);

    Owner->ForceNetUpdate();
}

void UFortHeldObjectComponent::DropHeldObject()
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || GetHeldObjectState() != EHeldObjectState::Held)
        return;

    Owner->FlushNetDormancy();

    DetachFromOwningPawn();

    GetHeldObjectState() = EHeldObjectState::Dropped;

    UnequipBackingWeapon();
    ClearOwningPawn();
    OnRep_OwningPawn(GetPreviousOwningPawn().Get());

    if (auto ProjectileMovementComponent = GetProjectileMovementComponent())
        ProjectileMovementComponent->SetVelocity(FVector(0.f, 0.f, 0.f));

    GetOnHeldObjectDropped().ProcessMulticastDelegate<UObject>(nullptr);

    Owner->ForceNetUpdate();
}

void UFortHeldObjectComponent::PlaceHeldObject()
{
    static auto PlacementForwardDistance = Anchor::Data<const float*>(TEXT("UFortHeldObjectComponent::UpdatePlacementPreview placement forward distance"))
        .String(TEXT("HeldObject.Placement.Mesh"))
        .Start()
        .Find("movss xmm1, dword ptr [0x*]; movaps xmm6, xmm0")
        .Target()
        .Get();

    static auto PlacementDownDistance = Anchor::Data<const float*>(TEXT("UFortHeldObjectComponent::UpdatePlacementPreview placement down distance"))
        .String(TEXT("HeldObject.Placement.Mesh"))
        .Start()
        .Find("subss xmm7, [0x*]")
        .Target()
        .Get();

    static auto PlacementTraceChannel = (ECollisionChannel)Anchor::Offset(TEXT("UFortHeldObjectComponent::UpdatePlacementPreview placement trace channel"))
        .String(TEXT("HeldObject.Placement.Mesh"))
        .Start()
        .Find("mov dword ptr [rsp+0x28], 0x*")
        .Imm();

    auto Owner = GetOwner();
    auto RootComponent = Owner ? Owner->GetRootComponent() : nullptr;
    auto PlayerPawn = GetOwningPawn();

    if (!RootComponent || !PlayerPawn || !Owner->HasAuthority() || GetHeldObjectState() != EHeldObjectState::Held || !PlacementForwardDistance || !PlacementDownDistance)
        return;

    const FVector Start = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * *PlacementForwardDistance;
    const FVector End(Start.X, Start.Y, Start.Z - *PlacementDownDistance);
    FHitResult Hit;

    if (!GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, PlacementTraceChannel, FCollisionShape::MakeSphere(RootComponent->CalcLocalBounds().SphereRadius)))
        return;

    Owner->FlushNetDormancy();

    DetachFromOwningPawn();

    RootComponent->K2_SetWorldLocationAndRotation(Hit.GetLocation(), FRotator{ 0.f, GetObjectPickupRotation().Yaw + (float)(int32)PlayerPawn->GetActorRotation().Yaw, 0.f }, false, true);

    GetHeldObjectState() = EHeldObjectState::Placed;

    UnequipBackingWeapon();
    ClearOwningPawn();
    OnRep_OwningPawn(GetPreviousOwningPawn().Get());

    GetOnHeldObjectPlaced().ProcessMulticastDelegate<UObject>(nullptr);

    Owner->ForceNetUpdate();
}

void UFortHeldObjectComponent::ThrowHeldObject(const FVector& DetachLocation, const FRotator& ThrowDirection)
{
    auto Owner = GetOwner();
    auto RootComponent = Owner ? Owner->GetRootComponent() : nullptr;

    if (!RootComponent || !Owner->HasAuthority() || GetHeldObjectState() != EHeldObjectState::Held)
        return;

    Owner->FlushNetDormancy();

    DetachFromOwningPawn();

    RootComponent->K2_SetWorldLocationAndRotation(DetachLocation, ThrowDirection, false, true);

    GetHeldObjectState() = EHeldObjectState::Thrown;

    ClearOwningPawn();
    OnRep_OwningPawn(GetPreviousOwningPawn().Get());

    if (auto ProjectileMovementComponent = GetProjectileMovementComponent())
    {
        ProjectileMovementComponent->SetProjectileGravityScale(GetGravityScale());
        ProjectileMovementComponent->SetVelocity(ThrowDirection.Vector() * GetMaxSpeed());
    }

    GetOnHeldObjectThrown().ProcessMulticastDelegate<UObject>(nullptr);

    Owner->ForceNetUpdate();
}

void UFortHeldObjectComponent::OnThrowComplete()
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || GetHeldObjectState() != EHeldObjectState::Thrown)
        return;

    UnequipBackingWeapon();
}

void UFortHeldObjectComponent::OnWeaponUnequipped()
{
    if (GetHeldObjectState() != EHeldObjectState::Held)
        return;

    SetDroppedFromWeaponSwap(true);
    DropHeldObject();
}

void UFortHeldObjectComponent::HandleOwnerDestroyed(AActor* DestroyedActor)
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority())
        return;

    if (GetGrantedWeaponItem().IsValid())
        UnequipBackingWeapon();

    if (GetHeldObjectState() == EHeldObjectState::Held)
        ClearOwningPawn();

    GetOnHeldObjectDestroy().ProcessMulticastDelegate<UObject>(nullptr);
}

void UFortHeldObjectComponent::HandleOwnerAsBuildingActorDestroyed(AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, const FVector& HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, const FVector& Momentum)
{
    HandleOwnerDestroyed(DamagedActor);
}

void UFortHeldObjectComponent::OnRep_OwningPawn(AFortPlayerPawn* LastOwningPawn)
{
    static auto OnRep_OwningPawn = Anchor::Function<void (*)(UFortHeldObjectComponent*, AFortPlayerPawn*)>(TEXT("UFortHeldObjectComponent::OnRep_OwningPawn"))
        .Exec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnRep_OwningPawn"))
        .Find("mov [*+0x20], *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnRep_OwningPawn"))
        .Get();

    OnRep_OwningPawn(this, LastOwningPawn);
}

float UFortHeldObjectComponent::GetGravityScale() const
{
    static auto GetGravityScale = Anchor::Function<float (*)(const UFortHeldObjectComponent*)>(TEXT("UFortHeldObjectComponent::GetGravityScale"))
        .Native(TEXT("/Script/FortniteGame.FortHeldObjectComponent:GetGravityScale"))
        .Get();

    return GetGravityScale(this);
}

AFortPlayerPawn* UFortHeldObjectComponent::GetInstigator() const
{
    static auto GetInstigator = Anchor::Function<AFortPlayerPawn* (*)(const UFortHeldObjectComponent*)>(TEXT("UFortHeldObjectComponent::GetInstigator"))
        .Native(TEXT("/Script/FortniteGame.FortHeldObjectComponent:GetInstigator"))
        .Get();

    return GetInstigator(this);
}

float UFortHeldObjectComponent::GetMaxSpeed() const
{
    static auto GetMaxSpeed = Anchor::Function<float (*)(const UFortHeldObjectComponent*)>(TEXT("UFortHeldObjectComponent::GetMaxSpeed"))
        .Native(TEXT("/Script/FortniteGame.FortHeldObjectComponent:GetMaxSpeed"))
        .Get();

    return GetMaxSpeed(this);
}

void UFortHeldObjectComponent::SetupOwningPawn(AFortPlayerPawn* NewOwningPawn)
{
    SetOwningPawn(NewOwningPawn);

    if (NewOwningPawn)
        NewOwningPawn->SetHeldObject(GetOwner());
}

void UFortHeldObjectComponent::ClearOwningPawn()
{
    auto OwningPawn = GetOwningPawn();

    if (OwningPawn)
        OwningPawn->SetHeldObject(nullptr);

    GetPreviousOwningPawn() = OwningPawn;
    SetOwningPawn(nullptr);
}

void UFortHeldObjectComponent::DetachFromOwningPawn()
{
    if (auto Owner = GetOwner())
        Owner->K2_DetachFromActor(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld);
}

void UFortHeldObjectComponent::EquipBackingWeapon()
{
    auto PlayerPawn = GetOwningPawn();
    auto PlayerController = PlayerPawn ? Cast<AFortPlayerControllerAthena>(PlayerPawn->GetController()) : nullptr;
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;
    auto WeaponDefinition = WorldInventory ? GetEquippedWeaponItemDefinition().LoadSynchronous() : nullptr;

    if (!WeaponDefinition)
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(WeaponDefinition, 1, 0);
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    if (auto WeaponStats = WeaponDefinition->GetWeaponStats<FFortBaseWeaponStats>())
        ItemEntry->SetLoadedAmmo(WeaponDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry->GetLevel()));

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;
    WorldInventory->SetItemRequiresUpdate(*ItemEntry);
    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
            GetGrantedWeaponItem() = Item;
    }

    GetGrantedWeapon() = PlayerPawn->EquipWeaponDefinition(WeaponDefinition, ItemGuid);

    FScriptDelegate WeaponUnequippedDelegate;
    WeaponUnequippedDelegate.BindUFunction(this, FName(TEXT("OnWeaponUnequipped")));

    PlayerPawn->GetOnWeaponUnEquippedDelegate().AddUnique(WeaponUnequippedDelegate);
}

void UFortHeldObjectComponent::UnequipBackingWeapon()
{
    auto GrantedWeapon = GetGrantedWeapon().Get();
    auto PlayerPawn = Cast<AFortPlayerPawn>(GrantedWeapon ? GrantedWeapon->GetOwner() : (AActor*)GetInstigator());
    auto PlayerController = PlayerPawn ? Cast<AFortPlayerController>(PlayerPawn->GetController()) : nullptr;
    auto GrantedWeaponItem = GetGrantedWeaponItem().Get();

    if (PlayerPawn)
    {
        FScriptDelegate WeaponUnequippedDelegate;
        WeaponUnequippedDelegate.BindUFunction(this, FName(TEXT("OnWeaponUnequipped")));

        PlayerPawn->GetOnWeaponUnEquippedDelegate().Remove(WeaponUnequippedDelegate);
    }

    if (PlayerController && GrantedWeaponItem)
    {
        const bool bWasCurrentWeapon = GrantedWeapon && PlayerPawn->GetCurrentWeapon() == GrantedWeapon;

        PlayerController->RemoveInventoryItem(GrantedWeaponItem->GetItemGuid(), 1, true);

        if (bWasCurrentWeapon)
            PlayerController->ForceEquipValidWeapon();
    }

    GetGrantedWeaponItem() = nullptr;
    GetGrantedWeapon() = nullptr;
}

void UFortHeldObjectComponent::execPickupHeldObject(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execPickupHeldObject, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PickupHeldObject"), &UFortHeldObjectComponent::execPickupHeldObject)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PickupHeldObject"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PickupHeldObject"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PickupHeldObject")));

    AFortPlayerPawn* PlayerPawn = nullptr;

    Stack.StepCompiledIn(&PlayerPawn);
    Stack.Code += !!Stack.Code;

    PickupHeldObject(PlayerPawn);
}

void UFortHeldObjectComponent::execDropHeldObject(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execDropHeldObject, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:DropHeldObject"), &UFortHeldObjectComponent::execDropHeldObject)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:DropHeldObject"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:DropHeldObject"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:DropHeldObject")));

    Stack.Code += !!Stack.Code;

    DropHeldObject();
}

void UFortHeldObjectComponent::execPlaceHeldObject(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execPlaceHeldObject, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PlaceHeldObject"), &UFortHeldObjectComponent::execPlaceHeldObject)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PlaceHeldObject"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PlaceHeldObject"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:PlaceHeldObject")));

    Stack.Code += !!Stack.Code;

    PlaceHeldObject();
}

void UFortHeldObjectComponent::execThrowHeldObject(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execThrowHeldObject, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:ThrowHeldObject"), &UFortHeldObjectComponent::execThrowHeldObject)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:ThrowHeldObject"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:ThrowHeldObject"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:ThrowHeldObject")));

    FVector DetachLocation;
    FRotator ThrowDirection;

    Stack.StepCompiledIn(&DetachLocation);
    Stack.StepCompiledIn(&ThrowDirection);
    Stack.Code += !!Stack.Code;

    ThrowHeldObject(DetachLocation, ThrowDirection);
}

void UFortHeldObjectComponent::execOnThrowComplete(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execOnThrowComplete, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnThrowComplete"), &UFortHeldObjectComponent::execOnThrowComplete)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnThrowComplete"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnThrowComplete"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnThrowComplete")));

    Stack.Code += !!Stack.Code;

    OnThrowComplete();
}

void UFortHeldObjectComponent::execOnWeaponUnequipped(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execOnWeaponUnequipped, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnWeaponUnequipped"), &UFortHeldObjectComponent::execOnWeaponUnequipped)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnWeaponUnequipped"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnWeaponUnequipped"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnWeaponUnequipped")));

    Stack.Code += !!Stack.Code;

    OnWeaponUnequipped();
}

void UFortHeldObjectComponent::execHandleOwnerDestroyed(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execHandleOwnerDestroyed, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerDestroyed"), &UFortHeldObjectComponent::execHandleOwnerDestroyed)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerDestroyed"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerDestroyed"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerDestroyed")));

    AActor* DestroyedActor = nullptr;

    Stack.StepCompiledIn(&DestroyedActor);
    Stack.Code += !!Stack.Code;

    HandleOwnerDestroyed(DestroyedActor);
}

void UFortHeldObjectComponent::execHandleOwnerAsBuildingActorDestroyed(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortHeldObjectComponent, execHandleOwnerAsBuildingActorDestroyed, Anchor::Hook(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerAsBuildingActorDestroyed"), &UFortHeldObjectComponent::execHandleOwnerAsBuildingActorDestroyed)
        .Body(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerAsBuildingActorDestroyed"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerAsBuildingActorDestroyed"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerAsBuildingActorDestroyed")));

    AActor* DamagedActor = nullptr;
    float Damage = 0.f;
    AController* InstigatedBy = nullptr;
    AActor* DamageCauser = nullptr;
    FVector HitLocation;
    UPrimitiveComponent* FHitComponent = nullptr;
    FName BoneName(NAME_None);
    FVector Momentum;

    Stack.StepCompiledIn(&DamagedActor);
    Stack.StepCompiledIn(&Damage);
    Stack.StepCompiledIn(&InstigatedBy);
    Stack.StepCompiledIn(&DamageCauser);
    Stack.StepCompiledIn(&HitLocation);
    Stack.StepCompiledIn(&FHitComponent);
    Stack.StepCompiledIn(&BoneName);
    Stack.StepCompiledIn(&Momentum);
    Stack.Code += !!Stack.Code;

    HandleOwnerAsBuildingActorDestroyed(DamagedActor, Damage, InstigatedBy, DamageCauser, HitLocation, FHitComponent, BoneName, Momentum);
}

TAssetPtr<UFortWeaponItemDefinition>& UFortHeldObjectComponent::GetEquippedWeaponItemDefinition()
{
    static auto EquippedWeaponItemDefinition = StaticClass()->FindPropertyByName(FName(TEXT("EquippedWeaponItemDefinition")));

    return *EquippedWeaponItemDefinition->ContainerPtrToValuePtr<TAssetPtr<UFortWeaponItemDefinition>>(this);
}

FMulticastScriptDelegate& UFortHeldObjectComponent::GetOnHeldObjectPickedUp()
{
    static auto OnHeldObjectPickedUp = StaticClass()->FindPropertyByName(FName(TEXT("OnHeldObjectPickedUp")));

    return *OnHeldObjectPickedUp->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

FMulticastScriptDelegate& UFortHeldObjectComponent::GetOnHeldObjectPlaced()
{
    static auto OnHeldObjectPlaced = StaticClass()->FindPropertyByName(FName(TEXT("OnHeldObjectPlaced")));

    return *OnHeldObjectPlaced->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

FMulticastScriptDelegate& UFortHeldObjectComponent::GetOnHeldObjectDropped()
{
    static auto OnHeldObjectDropped = StaticClass()->FindPropertyByName(FName(TEXT("OnHeldObjectDropped")));

    return *OnHeldObjectDropped->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

FMulticastScriptDelegate& UFortHeldObjectComponent::GetOnHeldObjectThrown()
{
    static auto OnHeldObjectThrown = StaticClass()->FindPropertyByName(FName(TEXT("OnHeldObjectThrown")));

    return *OnHeldObjectThrown->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

FMulticastScriptDelegate& UFortHeldObjectComponent::GetOnHeldObjectDestroy()
{
    static auto OnHeldObjectDestroy = StaticClass()->FindPropertyByName(FName(TEXT("OnHeldObjectDestroy")));

    return *OnHeldObjectDestroy->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

void UFortHeldObjectComponent::SetDroppedFromWeaponSwap(bool bInDroppedFromWeaponSwap)
{
    static auto bDroppedFromWeaponSwap = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bDroppedFromWeaponSwap")));

    bDroppedFromWeaponSwap->SetPropertyValue_InContainer(this, bInDroppedFromWeaponSwap);
}

EHeldObjectState& UFortHeldObjectComponent::GetHeldObjectState()
{
    static auto HeldObjectState = StaticClass()->FindPropertyByName(FName(TEXT("HeldObjectState")));

    return *HeldObjectState->ContainerPtrToValuePtr<EHeldObjectState>(this);
}

AFortPlayerPawn* UFortHeldObjectComponent::GetOwningPawn()
{
    static auto OwningPawn = StaticClass()->FindPropertyByName(FName(TEXT("OwningPawn")));
    static auto LastOwningPawn = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnRep_OwningPawn")))->FindPropertyByName(FName(TEXT("LastOwningPawn")));

    if (LastOwningPawn)
        return *OwningPawn->ContainerPtrToValuePtr<AFortPlayerPawn*>(this);

    return OwningPawn->ContainerPtrToValuePtr<TWeakObjectPtr<AFortPlayerPawn>>(this)->Get();
}

void UFortHeldObjectComponent::SetOwningPawn(AFortPlayerPawn* NewOwningPawn)
{
    static auto OwningPawn = StaticClass()->FindPropertyByName(FName(TEXT("OwningPawn")));
    static auto LastOwningPawn = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnRep_OwningPawn")))->FindPropertyByName(FName(TEXT("LastOwningPawn")));

    if (LastOwningPawn)
        *OwningPawn->ContainerPtrToValuePtr<AFortPlayerPawn*>(this) = NewOwningPawn;
    else
        *OwningPawn->ContainerPtrToValuePtr<TWeakObjectPtr<AFortPlayerPawn>>(this) = NewOwningPawn;
}

FRotator& UFortHeldObjectComponent::GetObjectPickupRotation()
{
    static auto ObjectPickupRotation = StaticClass()->FindPropertyByName(FName(TEXT("ObjectPickupRotation")));

    return *ObjectPickupRotation->ContainerPtrToValuePtr<FRotator>(this);
}

TWeakObjectPtr<AFortPlayerPawn>& UFortHeldObjectComponent::GetPreviousOwningPawn()
{
    static auto PreviousOwningPawn = StaticClass()->FindPropertyByName(FName(TEXT("PreviousOwningPawn")));

    return *PreviousOwningPawn->ContainerPtrToValuePtr<TWeakObjectPtr<AFortPlayerPawn>>(this);
}

TWeakObjectPtr<UFortWorldItem>& UFortHeldObjectComponent::GetGrantedWeaponItem()
{
    static auto GrantedWeaponItem = StaticClass()->FindPropertyByName(FName(TEXT("GrantedWeaponItem")));

    return *GrantedWeaponItem->ContainerPtrToValuePtr<TWeakObjectPtr<UFortWorldItem>>(this);
}

TWeakObjectPtr<AFortWeapon>& UFortHeldObjectComponent::GetGrantedWeapon()
{
    static auto GrantedWeapon = StaticClass()->FindPropertyByName(FName(TEXT("GrantedWeapon")));

    return *GrantedWeapon->ContainerPtrToValuePtr<TWeakObjectPtr<AFortWeapon>>(this);
}

UFortProjectileMovementComponent* UFortHeldObjectComponent::GetProjectileMovementComponent() const
{
    static auto ProjectileMovementComponent = StaticClass()->FindPropertyByName(FName(TEXT("ProjectileMovementComponent")));

    return *ProjectileMovementComponent->ContainerPtrToValuePtr<UFortProjectileMovementComponent*>(this);
}
