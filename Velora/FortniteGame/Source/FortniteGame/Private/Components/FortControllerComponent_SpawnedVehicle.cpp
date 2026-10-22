#include "pch.h"
#include "Components/FortControllerComponent_SpawnedVehicle.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Pawns/FortPlayerPawn.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortControllerComponent_SpawnedVehicle::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_SpawnedVehicle"));

    return Class;
}

void UFortControllerComponent_SpawnedVehicle::execExitSpawnedVehicle(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_SpawnedVehicle, execExitSpawnedVehicle, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortControllerComponent_SpawnedVehicle:ExitSpawnedVehicle"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortControllerComponent_SpawnedVehicle:ExitSpawnedVehicle")));

    Stack.Code += !!Stack.Code;

    ExitSpawnedVehicle();
}

AFortAthenaVehicle* UFortControllerComponent_SpawnedVehicle::SpawnVehicleAndEnter(UClass* VehicleClass, bool bSetPawnAsVehicleOwner, EVehicleSeats SeatToEnter, bool bIgnoreDBNOCheck)
{
    auto Controller = Cast<AController>(GetOwner());
    auto Pawn = Controller ? Cast<AFortPlayerPawn>(Controller->GetPawn()) : nullptr;
    auto World = GetWorld();

    if (!Pawn || !World || !VehicleClass)
        return GetSpawnedVehicle();

    if (GetSpawnedVehicle())
        ExitSpawnedVehicle();

    const FVector Location = Pawn->GetActorLocation();
    const FRotator Rotation = Pawn->GetActorRotation();

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = bSetPawnAsVehicleOwner ? (AActor*)Pawn : nullptr;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto Vehicle = Cast<AFortAthenaVehicle>(World->SpawnActor(VehicleClass, &Location, &Rotation, SpawnParameters));

    if (!Vehicle)
        return nullptr;

    if (!Vehicle->ForceEnterSeat(Pawn, SeatToEnter, bIgnoreDBNOCheck))
    {
        Vehicle->Destroy();
        return nullptr;
    }

    GetSpawnedVehicle() = Vehicle;

    return Vehicle;
}

void UFortControllerComponent_SpawnedVehicle::ExitSpawnedVehicle()
{
    auto Vehicle = GetSpawnedVehicle();

    if (!Vehicle)
        return;

    auto Controller = Cast<AController>(GetOwner());
    auto Pawn = Controller ? Cast<AFortPlayerPawn>(Controller->GetPawn()) : nullptr;

    if (Pawn && Pawn->GetVehicle() == Vehicle)
        Pawn->ServerOnExitVehicle(ETryExitVehicleBehavior::ForceAlways);

    GetSpawnedVehicle() = nullptr;

    Vehicle->Destroy();
}

AFortAthenaVehicle*& UFortControllerComponent_SpawnedVehicle::GetSpawnedVehicle()
{
    static auto SpawnedVehicle = StaticClass()->FindPropertyByName(FName(TEXT("SpawnedVehicle")));

    return *SpawnedVehicle->ContainerPtrToValuePtr<AFortAthenaVehicle*>(this);
}
