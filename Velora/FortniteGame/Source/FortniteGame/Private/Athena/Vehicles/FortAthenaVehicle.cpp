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
        auto ServerUpdatePhysicsParams = (UFunction*)StaticFindObject(nullptr, nullptr, GetServerFunctionPath(TEXT("ServerUpdatePhysicsParams")));

        if (!ServerUpdatePhysicsParams)
            return false;

        auto ServerUpdatePhysicsParams_Implementation = Scanner::Exec(GetServerFunctionPath(TEXT("ServerUpdatePhysicsParams"))).Find("call [rax+*]", 2).Slot(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"));
        auto ServerUpdateInputParams = Scanner::Exec(GetServerFunctionPath(TEXT("ServerUpdateInputParams")));
        auto ServerUpdateInputParams_Implementation = ServerUpdateInputParams.Get() ? ServerUpdateInputParams.Find("call [rax+*]", 2).Slot(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation")) : 0;
        auto OwnerClass = (UClass*)ServerUpdatePhysicsParams->GetOuter();
        auto OwnerVTable = *(void***)OwnerClass->GetDefaultObject(false);
        auto PhysicsStub = OwnerVTable[ServerUpdatePhysicsParams_Implementation / sizeof(void*)];
        auto InputStub = OwnerVTable[ServerUpdateInputParams_Implementation / sizeof(void*)];

        for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(OwnerClass))
                continue;

            if (auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false))
            {
                auto VTable = *(void***)DefaultObject;

                if (ServerUpdatePhysicsParams_Implementation && VTable[ServerUpdatePhysicsParams_Implementation / sizeof(void*)] == PhysicsStub)
                    DetourVTable(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"), VTable + ServerUpdatePhysicsParams_Implementation / sizeof(void*), &AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation);

                if (ServerUpdateInputParams_Implementation && VTable[ServerUpdateInputParams_Implementation / sizeof(void*)] == InputStub)
                    DetourVTable(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"), VTable + ServerUpdateInputParams_Implementation / sizeof(void*), &AFortAthenaVehicle::ServerUpdateInputParams_Implementation);
            }
        }

        return true;
    }();
}

const TCHAR* AFortAthenaVehicle::GetServerFunctionPath(const TCHAR* FunctionName)
{
    static auto bSMVehicle = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSMVehicle:ServerUpdatePhysicsParams")) != nullptr;
    static TCHAR Path[256];

    swprintf_s(Path, TEXT("/Script/FortniteGame.%s:%s"), bSMVehicle ? TEXT("FortAthenaSMVehicle") : TEXT("FortAthenaVehicle"), FunctionName);

    return Path;
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
    static auto WakeUp = []
    {
        auto OnRep_InputParams = Scanner::Exec(GetServerFunctionPath(TEXT("OnRep_InputParams"))).Find("jmp *");

        return (OnRep_InputParams.Is("jmp [rax+*]") ? OnRep_InputParams : Scanner::At(OnRep_InputParams.Target()).Find("jmp [rax+*]")).Slot(TEXT("AFortAthenaVehicle::WakeUp"));
    }();

    if (WakeUp)
        (*(void (**)(AFortAthenaVehicle*))(*(uint8_t**)this + WakeUp))(this);
}
