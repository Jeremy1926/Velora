#include "pch.h"
#include "Athena/Vehicles/FortOctopusVehicle.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto ServerUpdateTowhook = []
{
    auto ServerUpdateTowhook = Scanner::Exec(TEXT("/Script/FortniteGame.FortOctopusVehicle:ServerUpdateTowhook"));
    auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortOctopusVehicle"));
    auto Slot = ServerUpdateTowhook.Get() ? ServerUpdateTowhook.Find("call [*+0x*]", 2).Slot(TEXT("AFortOctopusVehicle::ServerUpdateTowhook_Implementation")) : 0;

    return Slot && Scanner::At(Scanner::Virtual<const void*>(DefaultObject, Slot)).Is("ret*") ? DetourVTable(TEXT("AFortOctopusVehicle::ServerUpdateTowhook_Implementation"), *(void***)DefaultObject, Slot, &AFortOctopusVehicle::ServerUpdateTowhook_Implementation) : nullptr;
}();

void AFortOctopusVehicle::ServerUpdateTowhook_Implementation(FVector_NetQuantizeNormal InNetTowhookAimDir)
{
    static auto NetTowhookAimDir = GetClass()->FindPropertyByName(FName(TEXT("NetTowhookAimDir")));

    *NetTowhookAimDir->ContainerPtrToValuePtr<FVector_NetQuantizeNormal>(this) = InNetTowhookAimDir;
}
