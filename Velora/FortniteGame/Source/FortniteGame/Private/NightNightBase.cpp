#include "pch.h"
#include "NightNightBase.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

void ANightNightBase::MakePreparationsForNightNight()
{
    static auto bPrepareClient = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bPrepareClient")));

    bPrepareClient->SetPropertyValue_InContainer(this, true);
}

void ANightNightBase::ReturnToNight()
{
    TArray<AActor*> PlayerControllers;
    UGameplayStatics::GetAllActorsOfClass(this, AFortPlayerControllerAthena::StaticClass(), PlayerControllers);

    for (auto PlayerController : PlayerControllers)
        ((AFortPlayerControllerAthena*)PlayerController)->ClientReportPhaseFound(TEXT("NoBernieNo"));
}

void ANightNightBase::execMakePreparationsForNightNight(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ANightNightBase, execMakePreparationsForNightNight, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.NightNightBase:MakePreparationsForNightNight"))
        .Find("jmp 0x*")
        .Target()
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.NightNightBase:MakePreparationsForNightNight"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.NightNightBase:MakePreparationsForNightNight")));

    Stack.Code += !!Stack.Code;

    MakePreparationsForNightNight();
}

void ANightNightBase::execReturnToNight(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ANightNightBase, execReturnToNight, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.NightNightBase:ReturnToNight"))
        .Find("jmp 0x*")
        .Target()
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.NightNightBase:ReturnToNight"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.NightNightBase:ReturnToNight")));

    Stack.Code += !!Stack.Code;

    ReturnToNight();
}
