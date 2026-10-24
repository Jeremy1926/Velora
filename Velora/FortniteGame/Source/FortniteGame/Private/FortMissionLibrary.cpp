#include "pch.h"
#include "FortMissionLibrary.h"
#include "Pawns/FortPlayerPawn.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

bool UFortMissionLibrary::TeleportPlayerPawn(const UObject* WorldContextObject, AFortPlayerPawn* PlayerPawn, const FVector& DestLocation, const FRotator& DestRotation, bool bIgnoreCollision, bool bIgnoreSupplementalKillVolumeSweep)
{
    return PlayerPawn && PlayerPawn->TeleportTo(DestLocation, DestRotation, false, bIgnoreCollision);
}

void UFortMissionLibrary::execTeleportPlayerPawn(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortMissionLibrary, execTeleportPlayerPawn, Anchor::Hook()
        .StubExec(TEXT("/Script/FortniteGame.FortMissionLibrary:TeleportPlayerPawn"))
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortMissionLibrary:TeleportPlayerPawn"))
        .Find("setnz *; add [*+0x20], *; mov rbx, [rsp+0x*]; mov [rdi], sil; add rsp, *")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortMissionLibrary:TeleportPlayerPawn")));

    static auto WorldContextObjectParam = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMissionLibrary:TeleportPlayerPawn")))->FindPropertyByName(FName(TEXT("WorldContextObject")));

    UObject* WorldContextObject = nullptr;
    AFortPlayerPawn* PlayerPawn = nullptr;
    TGameValue<FVector> DestLocation;
    TGameValue<FRotator> DestRotation;
    uint32 bIgnoreCollision = 0;
    uint32 bIgnoreSupplementalKillVolumeSweep = 0;

    if (WorldContextObjectParam)
        Stack.StepCompiledIn(&WorldContextObject);

    Stack.StepCompiledIn(&PlayerPawn);
    Stack.StepCompiledIn(&DestLocation);
    Stack.StepCompiledIn(&DestRotation);
    Stack.StepCompiledIn(&bIgnoreCollision);
    Stack.StepCompiledIn(&bIgnoreSupplementalKillVolumeSweep);
    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = TeleportPlayerPawn(WorldContextObject, PlayerPawn, DestLocation.Get(), DestRotation.Get(), !!bIgnoreCollision, !!bIgnoreSupplementalKillVolumeSweep);
}
