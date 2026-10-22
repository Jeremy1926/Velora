#include "pch.h"
#include "FortNevadaVehicle.h"
#include "Components/FortControllerComponent_SpawnedVehicle.h"
#include "Pawns/FortPlayerPawn.h"
#include "GameFramework/Controller.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortNevadaVehicle::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/NevadaRuntime.FortNevadaVehicle"));

    return Class;
}

bool AFortNevadaVehicle::AttachActorToTractorBeam(AActor* Actor)
{
    DETOUR(AFortNevadaVehicle, AttachActorToTractorBeam, Anchor::Hook()
        .If(StaticClass())
        .String(TEXT("&AFortNevadaVehicle::OnAttachedActorDied"))
        .Start()
        .Optional()
        .Inline());

    auto Pawn = HasAuthority() ? Cast<AFortPlayerPawn>(Actor) : nullptr;
    auto SpawnedVehicleComponent = Pawn && !Pawn->GetVehicle() ? GetSpawnedVehicleComponent(Pawn) : nullptr;
    auto SpawnedVehicle = SpawnedVehicleComponent && !SpawnedVehicleComponent->GetSpawnedVehicle() ? SpawnedVehicleComponent->SpawnVehicleAndEnter(GetAttachedPawnHiddenVehicle(), false, EVehicleSeats::Driver, true) : nullptr;

    const bool bAttached = Original(Actor);

    if (!bAttached && SpawnedVehicle)
        SpawnedVehicleComponent->ExitSpawnedVehicle();

    return bAttached;
}

void AFortNevadaVehicle::CleanupTractorBeamAttachments()
{
    DETOUR(AFortNevadaVehicle, CleanupTractorBeamAttachments, Anchor::Hook()
        .If(StaticClass())
        .String(TEXT("&AFortNevadaVehicle::OnAttachedActorDied"))
        .Start()
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov rcx, [rbp+0x*]; xor rcx, rsp")
        .Find("call 0x*")
        .Target()
        .Optional()
        .Inline());

    auto Pawn = Cast<AFortPlayerPawn>(GetTractorBeamAttachedActor());

    Original();

    if (auto SpawnedVehicleComponent = Pawn ? GetSpawnedVehicleComponent(Pawn) : nullptr)
        SpawnedVehicleComponent->ExitSpawnedVehicle();
}

UFortControllerComponent_SpawnedVehicle* AFortNevadaVehicle::GetSpawnedVehicleComponent(AActor* Actor)
{
    auto Pawn = Cast<AFortPlayerPawn>(Actor);
    auto Controller = Pawn ? Pawn->GetController() : nullptr;

    return Controller && UFortControllerComponent_SpawnedVehicle::StaticClass() ? Controller->FindComponentByClass<UFortControllerComponent_SpawnedVehicle>() : nullptr;
}

AActor* AFortNevadaVehicle::GetTractorBeamAttachedActor() const
{
    static auto TractorBeamAttachedActor = StaticClass()->FindPropertyByName(FName(TEXT("TractorBeamAttachedActor")));

    return *TractorBeamAttachedActor->ContainerPtrToValuePtr<AActor*>(this);
}

UClass* AFortNevadaVehicle::GetAttachedPawnHiddenVehicle() const
{
    static auto AttachedPawnHiddenVehicle = StaticClass()->FindPropertyByName(FName(TEXT("AttachedPawnHiddenVehicle")));

    return *AttachedPawnHiddenVehicle->ContainerPtrToValuePtr<UClass*>(this);
}
