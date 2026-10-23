#include "pch.h"
#include "Athena/AI/FortAthenaAISpawnerDataComponent_Vehicle.h"
#include "AI/FortAthenaAIBotController.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Pawns/FortPlayerPawn.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

void UFortAthenaAISpawnerDataComponent_Vehicle::PostOnSpawned(APawn* PawnAI)
{
    DETOUR(UFortAthenaAISpawnerDataComponent_Vehicle, PostOnSpawned, Anchor::Hook()
        .String(TEXT("UFortAthenaAISpawnerDataComponent_InventoryBase::OnSpawned Failed to initialize Inventory on Pawn=%s Controller=%s (Can't create/initialize inventory on AI)"))
        .Optional()
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_AIBotInventory"))
        .LogSlot(TEXT("UFortAthenaAISpawnerDataComponent::PostOnSpawned"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_Vehicle"), "ret*")
        .InVTable(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_Vehicle")));

    auto Pawn = Cast<AFortPlayerPawn>(PawnAI);
    auto BotController = Pawn ? Cast<AFortAthenaAIBotController>(Pawn->GetController()) : nullptr;
    auto World = Pawn ? Pawn->GetWorld() : nullptr;
    auto VehicleClass = GetVehicleToSpawnAndEnter();

    if (!BotController || !World || !VehicleClass)
        return;

    if (auto VehicleSkillSets = GetVehicleSkillSets())
    {
        BotController->SetVehicleSkillSets(*VehicleSkillSets);
        BotController->SetHaveInfiniteFuelOnVehicle(GetbSetInfiniteFuel());
    }

    const FVector Location = Pawn->GetActorLocation();
    const FRotator Rotation = Pawn->GetActorRotation();

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    auto Vehicle = Cast<AFortAthenaVehicle>(World->SpawnActor(VehicleClass, &Location, &Rotation, SpawnParameters));

    if (Vehicle && !Vehicle->ForceEnterSeat(Pawn, EVehicleSeats::Driver, false))
        Vehicle->Destroy();
}

UClass* UFortAthenaAISpawnerDataComponent_Vehicle::GetVehicleToSpawnAndEnter() const
{
    static auto VehicleToSpawnAndEnter = GetClass()->FindPropertyByName(FName(TEXT("VehicleToSpawnAndEnter")));

    return *VehicleToSpawnAndEnter->ContainerPtrToValuePtr<UClass*>(this);
}

bool UFortAthenaAISpawnerDataComponent_Vehicle::GetbSetInfiniteFuel() const
{
    static auto bSetInfiniteFuel = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bSetInfiniteFuel")));

    return bSetInfiniteFuel->GetPropertyValue_InContainer(this);
}

const TArray<UClass*>* UFortAthenaAISpawnerDataComponent_Vehicle::GetVehicleSkillSets() const
{
    static auto VehicleSkillSets = GetClass()->FindPropertyByName(FName(TEXT("VehicleSkillSets")));

    return VehicleSkillSets ? VehicleSkillSets->ContainerPtrToValuePtr<TArray<UClass*>>(this) : nullptr;
}
