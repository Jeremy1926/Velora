#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Components/PrimitiveComponent.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectArray.h"
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
        auto PhysicsFunctionName = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPhysicsPawn:ServerMove")) ? TEXT("ServerMove") : TEXT("ServerUpdatePhysicsParams");
        auto ServerUpdatePhysicsParams = (UFunction*)StaticFindObject(nullptr, nullptr, GetServerFunctionPath(PhysicsFunctionName));

        if (!ServerUpdatePhysicsParams)
            return false;

        auto ServerUpdatePhysicsParams_Implementation = Scanner::Exec(GetServerFunctionPath(PhysicsFunctionName)).Find("call [rax+*]", 2).Slot(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"));
        auto ServerUpdateInputParams = Scanner::Exec(GetServerFunctionPath(TEXT("ServerUpdateInputParams")));
        auto ServerUpdateInputParams_Implementation = ServerUpdateInputParams.Get() ? ServerUpdateInputParams.Find("call [rax+*]", 2).Slot(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation")) : 0;
        auto OwnerClass = (UClass*)ServerUpdatePhysicsParams->GetOuter();
        auto OwnerDefaultObject = OwnerClass->GetDefaultObject(false);
        auto PhysicsStub = Scanner::Virtual<const void*>(OwnerDefaultObject, ServerUpdatePhysicsParams_Implementation);
        auto InputStub = Scanner::Virtual<const void*>(OwnerDefaultObject, ServerUpdateInputParams_Implementation);

        for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(OwnerClass))
                continue;

            if (auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false))
            {
                auto VTable = *(void***)DefaultObject;

                if (PhysicsStub && Scanner::Virtual<const void*>(DefaultObject, ServerUpdatePhysicsParams_Implementation) == PhysicsStub)
                    DetourVTable(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"), VTable, ServerUpdatePhysicsParams_Implementation, &AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation);

                if (InputStub && Scanner::Virtual<const void*>(DefaultObject, ServerUpdateInputParams_Implementation) == InputStub)
                    DetourVTable(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"), VTable, ServerUpdateInputParams_Implementation, &AFortAthenaVehicle::ServerUpdateInputParams_Implementation);
            }
        }

        return true;
    }();
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

    auto Rotation = bEncodedRotation ? FQuat(InState.Rotation.X - 2.5f, InState.Rotation.Y / 0.3f, InState.Rotation.Z + 2.0f, InState.Rotation.W / -1.2f) : InState.Rotation;

    PrimitiveComponent->K2_SetWorldLocationAndRotation(InState.Translation, Rotation.Rotator(), false, true);
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
    static auto WakeUp = []
    {
        auto OnRep_InputParams = Scanner::Exec(GetServerFunctionPath(TEXT("OnRep_InputParams"))).Find("jmp *");

        return (OnRep_InputParams.Is("jmp [rax+*]") ? OnRep_InputParams : Scanner::At(OnRep_InputParams.Target()).Find("jmp [rax+*]")).Slot(TEXT("AFortAthenaVehicle::WakeUp"));
    }();

    Scanner::Virtual<void (*)(AFortAthenaVehicle*)>(this, WakeUp)(this);
}

AFortPlayerPawn* AFortAthenaVehicle::GetPawnAtSeat(int32 SeatIdx) const
{
    static auto Call = Scanner::Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat")).Find("setnz *").Find("mov [*+0x20], *").Find("call *");
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("AFortAthenaVehicle::GetPawnAtSeat")) : 0;
    static auto GetPawnAtSeat = Scanner::Function<AFortPlayerPawn* (*)(const AFortAthenaVehicle*, int32)>(Slot ? nullptr : Call.Target(TEXT("AFortAthenaVehicle::GetPawnAtSeat")));

    return (Slot ? Scanner::Virtual<decltype(GetPawnAtSeat)>(this, Slot) : GetPawnAtSeat)(this, SeatIdx);
}
