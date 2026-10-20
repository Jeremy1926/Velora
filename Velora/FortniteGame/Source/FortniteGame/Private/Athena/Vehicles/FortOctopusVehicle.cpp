#include "pch.h"
#include "Athena/Vehicles/FortOctopusVehicle.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

DETOUR(AFortOctopusVehicle, ServerUpdateTowhook_Implementation, Anchor::Hook()
    .Exec(TEXT("/Script/FortniteGame.FortOctopusVehicle:ServerUpdateTowhook"))
    .Optional()
    .Find("call [*+0x*]", 2)
    .LogSlot(TEXT("AFortOctopusVehicle::ServerUpdateTowhook_Implementation"))
    .IsVirtual(TEXT("/Script/FortniteGame.Default__FortOctopusVehicle"), "ret*")
    .Optional()
    .InVTable(TEXT("/Script/FortniteGame.Default__FortOctopusVehicle")))
void AFortOctopusVehicle::ServerUpdateTowhook_Implementation(FVector_NetQuantizeNormal InNetTowhookAimDir)
{
    static auto NetTowhookAimDir = GetClass()->FindPropertyByName(FName(TEXT("NetTowhookAimDir")));

    *NetTowhookAimDir->ContainerPtrToValuePtr<FVector_NetQuantizeNormal>(this) = InNetTowhookAimDir;
}
