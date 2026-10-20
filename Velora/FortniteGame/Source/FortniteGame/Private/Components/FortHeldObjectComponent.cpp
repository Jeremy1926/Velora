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

static auto HeldObjectNatives = []
{
    int32 Hooked = 0;

    for (auto [FunctionPath, ExecFunction] : {
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:PickupHeldObject"), (Native)&UFortHeldObjectComponent::execPickupHeldObject },
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:DropHeldObject"), (Native)&UFortHeldObjectComponent::execDropHeldObject },
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:PlaceHeldObject"), (Native)&UFortHeldObjectComponent::execPlaceHeldObject },
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:ThrowHeldObject"), (Native)&UFortHeldObjectComponent::execThrowHeldObject },
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnThrowComplete"), (Native)&UFortHeldObjectComponent::execOnThrowComplete },
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnWeaponUnequipped"), (Native)&UFortHeldObjectComponent::execOnWeaponUnequipped },
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerDestroyed"), (Native)&UFortHeldObjectComponent::execHandleOwnerDestroyed },
        std::pair{ TEXT("/Script/FortniteGame.FortHeldObjectComponent:HandleOwnerAsBuildingActorDestroyed"), (Native)&UFortHeldObjectComponent::execHandleOwnerAsBuildingActorDestroyed } })
    {
        auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, FunctionPath);

        if (!Function)
            continue;

        auto Finish = Scanner::Exec(FunctionPath).Find("mov [*+0x20], *; jmp 0x*");
        auto Body = Finish.Get() ? Scanner::At(Finish.Find("jmp 0x*").Target()) : Scanner::Native(FunctionPath);

        if (!Body.Is("ret*"))
            continue;

        UE_LOG(LogVelora, Log, TEXT("Hooked %s, was 0x%llX"), FunctionPath, std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

        Function->SetNativeFunc(ExecFunction);
        Hooked++;
    }

    return Hooked;
}();

static Scanner::FCode UpdatePlacementPreview()
{
    static auto UpdatePlacementPreview = Scanner::String(TEXT("HeldObject.Placement.Mesh")).Start();

    return UpdatePlacementPreview;
}

UClass* UFortHeldObjectComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortHeldObjectComponent"));

    return Class;
}

void UFortHeldObjectComponent::PickupHeldObject(AFortPlayerPawn* PlayerPawn)
{
    auto Owner = GetOwner();

    if (!PlayerPawn || !Owner || !Owner->HasAuthority() || GetOwningPawn().IsValid())
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
    OnRep_OwningPawn();

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
    OnRep_OwningPawn();

    if (auto ProjectileMovementComponent = GetProjectileMovementComponent())
        ProjectileMovementComponent->SetVelocity(FVector(0.f, 0.f, 0.f));

    GetOnHeldObjectDropped().ProcessMulticastDelegate<UObject>(nullptr);

    Owner->ForceNetUpdate();
}

void UFortHeldObjectComponent::PlaceHeldObject()
{
    static auto PlacementForwardDistance = (const float*)UpdatePlacementPreview().Find("movss xmm1, dword ptr [0x*]; movaps xmm6, xmm0").Target(TEXT("UFortHeldObjectComponent::UpdatePlacementPreview placement forward distance"));
    static auto PlacementDownDistance = (const float*)UpdatePlacementPreview().Find("subss xmm7, [0x*]").Target(TEXT("UFortHeldObjectComponent::UpdatePlacementPreview placement down distance"));
    static auto PlacementTraceChannel = (ECollisionChannel)UpdatePlacementPreview().Find("mov dword ptr [rsp+0x28], 0x*").Imm(TEXT("UFortHeldObjectComponent::UpdatePlacementPreview placement trace channel"));

    auto Owner = GetOwner();
    auto RootComponent = Owner ? Owner->GetRootComponent() : nullptr;
    auto PlayerPawn = GetOwningPawn().Get();

    if (!RootComponent || !PlayerPawn || !Owner->HasAuthority() || GetHeldObjectState() != EHeldObjectState::Held || !PlacementForwardDistance || !PlacementDownDistance)
        return;

    const FVector Start = PlayerPawn->GetActorLocation() + PlayerPawn->GetActorForwardVector() * *PlacementForwardDistance;
    const FVector End(Start.X, Start.Y, Start.Z - *PlacementDownDistance);
    FHitResult Hit;

    if (!GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, PlacementTraceChannel, FCollisionShape::MakeSphere(RootComponent->CalcLocalBounds().SphereRadius)))
        return;

    Owner->FlushNetDormancy();

    DetachFromOwningPawn();

    RootComponent->K2_SetWorldLocationAndRotation(Hit.Location, FRotator{ 0.f, GetObjectPickupRotation().Yaw + (float)(int32)PlayerPawn->GetActorRotation().Yaw, 0.f }, false, true);

    GetHeldObjectState() = EHeldObjectState::Placed;

    UnequipBackingWeapon();
    ClearOwningPawn();
    OnRep_OwningPawn();

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
    OnRep_OwningPawn();

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

void UFortHeldObjectComponent::OnRep_OwningPawn()
{
    static auto OnRep_OwningPawn = Scanner::Function<void (*)(UFortHeldObjectComponent*)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortHeldObjectComponent:OnRep_OwningPawn")).Find("mov [*+0x20], *; jmp 0x*").Find("jmp 0x*").Target(TEXT("UFortHeldObjectComponent::OnRep_OwningPawn")));

    OnRep_OwningPawn(this);
}

float UFortHeldObjectComponent::GetGravityScale() const
{
    static auto GetGravityScale = Scanner::Function<float (*)(const UFortHeldObjectComponent*)>(Scanner::Native(TEXT("/Script/FortniteGame.FortHeldObjectComponent:GetGravityScale")).Get(TEXT("UFortHeldObjectComponent::GetGravityScale")));

    return GetGravityScale(this);
}

AFortPlayerPawn* UFortHeldObjectComponent::GetInstigator() const
{
    static auto GetInstigator = Scanner::Function<AFortPlayerPawn* (*)(const UFortHeldObjectComponent*)>(Scanner::Native(TEXT("/Script/FortniteGame.FortHeldObjectComponent:GetInstigator")).Get(TEXT("UFortHeldObjectComponent::GetInstigator")));

    return GetInstigator(this);
}

float UFortHeldObjectComponent::GetMaxSpeed() const
{
    static auto GetMaxSpeed = Scanner::Function<float (*)(const UFortHeldObjectComponent*)>(Scanner::Native(TEXT("/Script/FortniteGame.FortHeldObjectComponent:GetMaxSpeed")).Get(TEXT("UFortHeldObjectComponent::GetMaxSpeed")));

    return GetMaxSpeed(this);
}

void UFortHeldObjectComponent::SetupOwningPawn(AFortPlayerPawn* NewOwningPawn)
{
    GetOwningPawn() = NewOwningPawn;

    if (NewOwningPawn)
        NewOwningPawn->SetHeldObject(GetOwner());
}

void UFortHeldObjectComponent::ClearOwningPawn()
{
    if (auto OwningPawn = GetOwningPawn().Get())
        OwningPawn->SetHeldObject(nullptr);

    GetPreviousOwningPawn() = GetOwningPawn();
    GetOwningPawn() = nullptr;
}

void UFortHeldObjectComponent::DetachFromOwningPawn()
{
    if (auto Owner = GetOwner())
        Owner->K2_DetachFromActor(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld);
}

void UFortHeldObjectComponent::EquipBackingWeapon()
{
    auto PlayerPawn = GetOwningPawn().Get();
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
    AFortPlayerPawn* PlayerPawn = nullptr;

    Stack.StepCompiledIn(&PlayerPawn);
    Stack.Code += !!Stack.Code;

    PickupHeldObject(PlayerPawn);
}

void UFortHeldObjectComponent::execDropHeldObject(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    DropHeldObject();
}

void UFortHeldObjectComponent::execPlaceHeldObject(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    PlaceHeldObject();
}

void UFortHeldObjectComponent::execThrowHeldObject(FFrame& Stack, void* const Z_Param__Result)
{
    FVector DetachLocation;
    FRotator ThrowDirection;

    Stack.StepCompiledIn(&DetachLocation);
    Stack.StepCompiledIn(&ThrowDirection);
    Stack.Code += !!Stack.Code;

    ThrowHeldObject(DetachLocation, ThrowDirection);
}

void UFortHeldObjectComponent::execOnThrowComplete(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    OnThrowComplete();
}

void UFortHeldObjectComponent::execOnWeaponUnequipped(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    OnWeaponUnequipped();
}

void UFortHeldObjectComponent::execHandleOwnerDestroyed(FFrame& Stack, void* const Z_Param__Result)
{
    AActor* DestroyedActor = nullptr;

    Stack.StepCompiledIn(&DestroyedActor);
    Stack.Code += !!Stack.Code;

    HandleOwnerDestroyed(DestroyedActor);
}

void UFortHeldObjectComponent::execHandleOwnerAsBuildingActorDestroyed(FFrame& Stack, void* const Z_Param__Result)
{
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

TWeakObjectPtr<AFortPlayerPawn>& UFortHeldObjectComponent::GetOwningPawn()
{
    static auto OwningPawn = StaticClass()->FindPropertyByName(FName(TEXT("OwningPawn")));

    return *OwningPawn->ContainerPtrToValuePtr<TWeakObjectPtr<AFortPlayerPawn>>(this);
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
