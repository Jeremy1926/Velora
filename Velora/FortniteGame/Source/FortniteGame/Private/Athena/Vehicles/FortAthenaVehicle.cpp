#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Components/PrimitiveComponent.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectArray.h"
#include "Misc/EngineVersion.h"
#include "Detour.h"
#include "Scanner.h"

UClass* AFortAthenaVehicle::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicle"));

    return Class;
}

void AFortAthenaVehicle::InitializeServerImplementations()
{
    static auto ServerUpdatePhysicsParams = (UFunction*)StaticFindObject(nullptr, nullptr, GetServerFunctionPath(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPhysicsPawn:ServerMove")) ? TEXT("ServerMove") : TEXT("ServerUpdatePhysicsParams")));

    if (!ServerUpdatePhysicsParams)
        return;

    static auto ServerUpdatePhysicsParams_Implementation = THook(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"), &AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation)
        .Exec(ServerUpdatePhysicsParams)
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"))
        .Or()
        .Exec(ServerUpdatePhysicsParams)
        .Find("call [rax+*]")
        .LogSlot(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"))
        .InChildVTables((UClass*)ServerUpdatePhysicsParams->GetOuter());

    static auto ServerUpdateInputParams_Implementation = THook(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"), &AFortAthenaVehicle::ServerUpdateInputParams_Implementation)
        .Exec(GetServerFunctionPath(TEXT("ServerUpdateInputParams")))
        .Optional()
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"))
        .InChildVTables((UClass*)ServerUpdatePhysicsParams->GetOuter());
}

const TCHAR* AFortAthenaVehicle::GetServerFunctionPath(const TCHAR* FunctionName)
{
    static auto Owner = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPhysicsPawn:ServerUpdatePhysicsParams")) || StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPhysicsPawn:ServerMove")) ? TEXT("FortPhysicsPawn") : StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSMVehicle:ServerUpdatePhysicsParams")) ? TEXT("FortAthenaSMVehicle") : TEXT("FortAthenaVehicle");
    static TCHAR Path[256];

    swprintf_s(Path, TEXT("/Script/FortniteGame.%s:%s"), Owner, FunctionName);

    return Path;
}

void AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation(const FReplicatedAthenaVehiclePhysicsState& InState)
{
    static auto bEncodedRotation = []
    {
        auto PhysicsState = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ReplicatedPhysicsPawnState"));

        if (!PhysicsState)
            PhysicsState = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ReplicatedAthenaVehiclePhysicsState"));

        return PhysicsState->FindPropertyByName(FName(TEXT("SyncKey"))) != nullptr;
    }();

    auto PrimitiveComponent = Cast<UPrimitiveComponent>(GetRootComponent());

    if (!PrimitiveComponent)
        return;

    auto& Version = FEngineVersion::Current();
    auto Rotation = !bEncodedRotation ? InState.Rotation : Version.GetMajor() > 4 || Version.GetMinor() >= 25 ? FQuat(InState.Rotation.X - 0.3f, InState.Rotation.Y / -0.75f, InState.Rotation.Z + 0.15f, InState.Rotation.W / 1.1f) : FQuat(InState.Rotation.X - 2.5f, InState.Rotation.Y / 0.3f, InState.Rotation.Z + 2.0f, InState.Rotation.W / -1.2f);

    PrimitiveComponent->K2_SetWorldLocationAndRotation(InState.Translation, Rotation.Rotator(), false, true);
    PrimitiveComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, false, NAME_None);
    PrimitiveComponent->SetPhysicsAngularVelocityInRadians(InState.AngularVelocity, false, NAME_None);
}

void AFortAthenaVehicle::ServerUpdateInputParams_Implementation(const FReplicatedAthenaVehicleInputState& InState)
{
    static auto ReplicatedInputParams = GetClass()->FindPropertyByName(FName(TEXT("ReplicatedInputParams")));
    static auto LocalInputParams = GetClass()->FindPropertyByName(FName(TEXT("LocalInputParams")));

    memcpy(ReplicatedInputParams->ContainerPtrToValuePtr<void>(this), &InState, ReplicatedInputParams->GetSize());
    memcpy(LocalInputParams->ContainerPtrToValuePtr<void>(this), &InState, LocalInputParams->GetSize());

    WakeUp();
}

void AFortAthenaVehicle::WakeUp()
{
    static auto WakeUp = Anchor::VirtualFunction<void (*)(AFortAthenaVehicle*)>(TEXT("AFortAthenaVehicle::WakeUp"))
        .Exec(GetServerFunctionPath(TEXT("OnRep_InputParams")))
        .Find("jmp *")
        .Is("jmp [rax+*]")
        .Or()
        .Exec(GetServerFunctionPath(TEXT("OnRep_InputParams")))
        .Find("jmp *")
        .Target()
        .Find("jmp [rax+*]")
        .Get();

    WakeUp(this);
}

AFortPlayerPawn* AFortAthenaVehicle::GetPawnAtSeat(int32 SeatIdx) const
{
    static auto VirtualGetPawnAtSeat = Anchor::VirtualFunction<AFortPlayerPawn* (*)(const AFortAthenaVehicle*, int32)>(TEXT("AFortAthenaVehicle::GetPawnAtSeat"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call [*]")
        .Optional()
        .Get();

    if (VirtualGetPawnAtSeat)
        return VirtualGetPawnAtSeat(this, SeatIdx);

    static auto GetPawnAtSeat = Anchor::Function<AFortPlayerPawn* (*)(const AFortAthenaVehicle*, int32)>(TEXT("AFortAthenaVehicle::GetPawnAtSeat"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    return GetPawnAtSeat(this, SeatIdx);
}
