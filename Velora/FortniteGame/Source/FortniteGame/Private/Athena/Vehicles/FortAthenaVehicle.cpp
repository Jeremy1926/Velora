#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Components/PrimitiveComponent.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Detour.h"
#include "Scanner.h"

UClass* AFortAthenaVehicle::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicle"));

    return Class;
}

void AFortAthenaVehicle::InitializeServerImplementations()
{
    static auto ServerImplementations = []
    {
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortAthenaVehicle"));
        auto ServerUpdatePhysicsParams_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:ServerUpdatePhysicsParams")).Find("call [rax+*]", 2).Slot(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"));
        auto ServerUpdateInputParams_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:ServerUpdateInputParams")).Find("call [rax+*]", 2).Slot(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"));

        if (ServerUpdatePhysicsParams_Implementation)
            DetourVTable(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"), (void**)(VTable + ServerUpdatePhysicsParams_Implementation), &AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation);

        if (ServerUpdateInputParams_Implementation)
            DetourVTable(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"), (void**)(VTable + ServerUpdateInputParams_Implementation), &AFortAthenaVehicle::ServerUpdateInputParams_Implementation);

        return true;
    }();
}

void AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation(const FReplicatedAthenaVehiclePhysicsState& InState)
{
    auto PrimitiveComponent = Cast<UPrimitiveComponent>(GetRootComponent());

    if (!PrimitiveComponent)
        return;

    PrimitiveComponent->K2_SetWorldLocationAndRotation(InState.Translation, InState.Rotation.Rotator(), false, true);
    PrimitiveComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, false, NAME_None);
    PrimitiveComponent->SetPhysicsAngularVelocityInRadians(InState.AngularVelocity, false, NAME_None);
}

void AFortAthenaVehicle::ServerUpdateInputParams_Implementation(const FReplicatedAthenaVehicleInputState& InState)
{
    static auto ReplicatedInputParams = GetClass()->FindPropertyByName(FName(TEXT("ReplicatedInputParams")));
    static auto LocalInputParams = GetClass()->FindPropertyByName(FName(TEXT("LocalInputParams")));

    memcpy(ReplicatedInputParams->ContainerPtrToValuePtr<void>(this), &InState, ReplicatedInputParams->ElementSize);
    memcpy(LocalInputParams->ContainerPtrToValuePtr<void>(this), &InState, LocalInputParams->ElementSize);

    WakeUp();
}

void AFortAthenaVehicle::WakeUp()
{
    static auto WakeUp = Scanner::Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_InputParams")).Find("jmp [rax+*]").Slot(TEXT("AFortAthenaVehicle::WakeUp"));

    if (WakeUp)
        (*(void (**)(AFortAthenaVehicle*))(*(uint8_t**)this + WakeUp))(this);
}
