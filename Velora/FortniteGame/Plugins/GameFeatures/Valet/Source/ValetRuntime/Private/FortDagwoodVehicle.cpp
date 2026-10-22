#include "pch.h"
#include "FortDagwoodVehicle.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortDagwoodVehicle::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ValetRuntime.FortDagwoodVehicle"));

    return Class;
}

void AFortDagwoodVehicle::MakeInoperable()
{
    static auto bIsInoperable = StaticClass()->FindPropertyByName(FName(TEXT("bIsInoperable")));

    *bIsInoperable->ContainerPtrToValuePtr<bool>(this) = true;

    OnRep_IsInoperable();
    ForceNetUpdate();
}

bool AFortDagwoodVehicle::IsInoperable() const
{
    static auto bIsInoperable = StaticClass()->FindPropertyByName(FName(TEXT("bIsInoperable")));

    return *bIsInoperable->ContainerPtrToValuePtr<bool>(this);
}

void AFortDagwoodVehicle::AddSpawningFuel()
{
    static auto AddSpawningFuel = Anchor::Function<void (*)(AFortDagwoodVehicle*)>(TEXT("AFortDagwoodVehicle::AddSpawningFuel"))
        .Native(TEXT("/Script/ValetRuntime.FortDagwoodVehicleManager:OnVehicleSpawned"))
        .Is("push rbx; sub rsp, 0x20; mov rbx, rdx; test rdx, rdx; jz *; call 0x*; mov rcx, [rbx+0x10]; mov rdx, rax; call 0x*")
        .Find("test rbx, rbx; jz *; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/ValetRuntime.FortDagwoodVehicleManager:OnVehicleSpawned"))
        .Find("call 0x*", 2)
        .Target()
        .Get();

    AddSpawningFuel(this);
}

void AFortDagwoodVehicle::OnRep_IsInoperable()
{
    static auto OnRep_IsInoperable = Anchor::Function<void (*)(AFortDagwoodVehicle*)>(TEXT("AFortDagwoodVehicle::OnRep_IsInoperable"))
        .Body(TEXT("/Script/ValetRuntime.FortDagwoodVehicle:OnRep_IsInoperable"))
        .Get();

    OnRep_IsInoperable(this);
}
