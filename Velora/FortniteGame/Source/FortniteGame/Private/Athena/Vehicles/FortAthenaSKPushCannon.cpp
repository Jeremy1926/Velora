#include "pch.h"
#include "Athena/Vehicles/FortAthenaSKPushCannon.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Building/BuildingActor.h"
#include "Pawns/FortPlayerPawn.h"
#include "FortKismetLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

struct PushCannon_C_eventPawnLaunchedCosmetics_Parms
{
    AFortPlayerPawn* FiredPlayer;
};

UClass* AFortAthenaSKPushCannon::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSKPushCannon"));

    return Class;
}

void AFortAthenaSKPushCannon::ServerLaunchActorInBarrel(const FVector& LaunchDir)
{
    if (!HasAuthority())
        return;

    auto Pawn = GetPawnAtSeat(1);

    if (!Pawn)
        return;

    const FVector SafeLaunchDir = LaunchDir.GetSafeNormal();

    OnPreLaunchPawn(Pawn, SafeLaunchDir);
    Pawn->ServerOnExitVehicle(ETryExitVehicleBehavior::ForceOnBlocking);
    OnLaunchPawn(Pawn, SafeLaunchDir);
    MultiCastPushCannonLaunchedPlayer();
}

void AFortAthenaSKPushCannon::OnPreLaunchPawn(AFortPlayerPawn* Pawn, const FVector& LaunchDir)
{
    static auto bTraceFromExitSocket = GetClass()->FindPropertyByName(FName(TEXT("TraceStartLoc"))) != nullptr;

    if (GetPrelaunchSphereTraceEnabled().GetValueAtLevel(0.f) <= 0.f)
        return;

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(ObjectTypeQuery1);

    TArray<AActor*> ActorsToIgnore;
    TArray<FHitResult> OutHits;

    if (!bTraceFromExitSocket)
    {
        const FVector Start = GetSkeletalMesh()->GetSocketLocation(FName(TEXT("Muzzle")));

        UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, Start + LaunchDir * GetPrelaunchSphereTraceDistance().GetValueAtLevel(0.f), GetPrelaunchSphereTraceRadius().GetValueAtLevel(0.f), ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true, FLinearColor(1.f, 0.f, 0.f, 1.f), FLinearColor(0.f, 1.f, 0.f, 1.f), 5.f);
        PrelaunchDestroyBuildings(OutHits, Pawn);

        return;
    }

    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));

    auto VehicleInterface = (IFortVehicleInterface*)GetInterfaceAddress(FortVehicleInterface);
    const FVector TraceStartLoc = VehicleInterface->GetExitSocketTransform(1, 0).GetTranslation();
    const FVector TraceEndLoc = VehicleInterface->GetExitSocketTransform(1, 0).GetTranslation() + LaunchDir * GetPrelaunchSphereTraceDistance().GetValueAtLevel(0.f);
    auto CapsuleComponent = Pawn->GetCapsuleComponent();
    const float TraceForWallsHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();
    const float TraceForWallsRadius = CapsuleComponent->GetUnscaledCapsuleRadius() > GetPrelaunchSphereTraceRadius().GetValueAtLevel(0.f) ? CapsuleComponent->GetUnscaledCapsuleRadius() : GetPrelaunchSphereTraceRadius().GetValueAtLevel(0.f);

    if (UKismetSystemLibrary::CapsuleTraceMultiForObjects(this, TraceStartLoc, TraceEndLoc, TraceForWallsRadius, TraceForWallsHalfHeight, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, OutHits, true, FLinearColor(1.f, 0.f, 0.f, 1.f), FLinearColor(0.f, 1.f, 0.f, 1.f), 5.f))
        PrelaunchDestroyBuildings(OutHits, Pawn);
}

void AFortAthenaSKPushCannon::OnLaunchPawn(AFortPlayerPawn* Pawn, const FVector& LaunchDir)
{
    static auto bAddIgnoredActor = GetClass()->FindPropertyByName(FName(TEXT("TraceStartLoc"))) != nullptr;
    static auto PushCannonLaunched = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/Items/Weapons/Vehicles/GE_Athena_PushCannon_Launched.GE_Athena_PushCannon_Launched_C"));

    if (bAddIgnoredActor && HasAuthority())
        Pawn->AddIgnoredActor(this, 0.5f);

    Pawn->LaunchCharacterJump(LaunchDir * GetPlayerLaunchSpeed().GetValueAtLevel(0.f), true, true, true, true);
    UFortKismetLibrary::ApplyGlobalEnvironmentGameplayEffectToActor(Pawn, PushCannonLaunched, 0, FGameplayTagContainer());
    PawnLaunchedCosmetics(Pawn);
}

void AFortAthenaSKPushCannon::MultiCastPushCannonLaunchedPlayer()
{
    static auto MultiCastPushCannonLaunchedPlayer = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSKPushCannon:MultiCastPushCannonLaunchedPlayer"));

    CallRemoteFunction(MultiCastPushCannonLaunchedPlayer, nullptr, nullptr, nullptr);
    MultiCastPushCannonLaunchedPlayer_Implementation();
}

void AFortAthenaSKPushCannon::MultiCastPushCannonLaunchedPlayer_Implementation()
{
    static auto MultiCastPushCannonLaunchedPlayer_Implementation = Anchor::Slot(TEXT("AFortAthenaSKPushCannon::MultiCastPushCannonLaunchedPlayer_Implementation"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaSKPushCannon:MultiCastPushCannonLaunchedPlayer"))
        .Find("jmp [*+0x*]")
        .Get();

    Scanner::Virtual<void (*)(AFortAthenaSKPushCannon*)>(this, MultiCastPushCannonLaunchedPlayer_Implementation)(this);
}

void AFortAthenaSKPushCannon::PrelaunchDestroyBuildings(const TArray<FHitResult>& HitActors, AFortPlayerPawn* LaunchedPawn)
{
    static auto bBootyIsNotDestroyed = GetClass()->FindPropertyByName(FName(TEXT("TraceStartLoc"))) == nullptr;
    static auto BP_DeimosRift_C = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/Deimos/Spawners/RiftSpawners/BP_DeimosRift.BP_DeimosRift_C"));
    static auto Prop_Athena_Booty_C = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/Items/ForagedItems/Booty/Prop_Athena_Booty.Prop_Athena_Booty_C"));
    static auto AthenaBarrierObjective = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaBarrierObjective"));

    for (auto& HitResult : HitActors)
    {
        auto HitActor = HitResult.GetActor().Get();

        if (!HitActor || !HitActor->CanBeDamaged() || HitActor->IsA(BP_DeimosRift_C) || !HitActor->IsA(ABuildingActor::StaticClass()))
            continue;

        if ((bBootyIsNotDestroyed && HitActor->IsA(Prop_Athena_Booty_C)) || HitActor->IsA(AthenaBarrierObjective))
            continue;

        if (auto AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(LaunchedPawn))
            AbilitySystemComponent->BP_ApplyGameplayEffectToTarget(GetBuildingDestroyGE(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor), 0.f, FGameplayEffectContextHandle());

        LaunchedPawn->GetMesh()->IgnoreActorWhenMoving(HitActor, true);
    }
}

void AFortAthenaSKPushCannon::PawnLaunchedCosmetics(AFortPlayerPawn* FiredPlayer)
{
    static auto PawnLaunchedCosmetics = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C:PawnLaunchedCosmetics"));

    PushCannon_C_eventPawnLaunchedCosmetics_Parms Parms{ FiredPlayer };

    if (PawnLaunchedCosmetics)
        CallRemoteFunction(PawnLaunchedCosmetics, &Parms, nullptr, nullptr);
}

USkeletalMeshComponent* AFortAthenaSKPushCannon::GetSkeletalMesh() const
{
    static auto SkeletalMesh = GetClass()->FindPropertyByName(FName(TEXT("SkeletalMesh")));

    return *SkeletalMesh->ContainerPtrToValuePtr<USkeletalMeshComponent*>(this);
}

FScalableFloat& AFortAthenaSKPushCannon::GetPlayerLaunchSpeed()
{
    static auto PlayerLaunchSpeed = GetClass()->FindPropertyByName(FName(TEXT("PlayerLaunchSpeed")));

    return *PlayerLaunchSpeed->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat& AFortAthenaSKPushCannon::GetPrelaunchSphereTraceEnabled()
{
    static auto PrelaunchSphereTraceEnabled = GetClass()->FindPropertyByName(FName(TEXT("PrelaunchSphereTraceEnabled")));

    return *PrelaunchSphereTraceEnabled->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat& AFortAthenaSKPushCannon::GetPrelaunchSphereTraceDistance()
{
    static auto PrelaunchSphereTraceDistance = GetClass()->FindPropertyByName(FName(TEXT("PrelaunchSphereTraceDistance")));

    return *PrelaunchSphereTraceDistance->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat& AFortAthenaSKPushCannon::GetPrelaunchSphereTraceRadius()
{
    static auto PrelaunchSphereTraceRadius = GetClass()->FindPropertyByName(FName(TEXT("PrelaunchSphereTraceRadius")));

    return *PrelaunchSphereTraceRadius->ContainerPtrToValuePtr<FScalableFloat>(this);
}

TSubclassOf<UGameplayEffect>& AFortAthenaSKPushCannon::GetBuildingDestroyGE()
{
    static auto BuildingDestroyGE = GetClass()->FindPropertyByName(FName(TEXT("BuildingDestroyGE")));

    return *BuildingDestroyGE->ContainerPtrToValuePtr<TSubclassOf<UGameplayEffect>>(this);
}
