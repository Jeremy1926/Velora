#include "pch.h"
#include "FortCheatManager.h"
#include "Athena/FortGameModeAthena.h"
#include "Engine/World.h"
#include "Misc/Parse.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortCheatManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortCheatManager"));

    return Class;
}

bool UFortCheatManager::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
    DETOUR(UFortCheatManager, ProcessConsoleExec, Anchor::Hook()
        .String(TEXT("NotifyEventTriggered: Unable to find function '%s'"))
        .Start()
        .Find("call [*+0x*]")
        .Entry(TEXT("/Script/CoreUObject.Default__Object"))
        .Find("call [*+0x*]; mov *, eax; test al, 0x1; jnz *")
        .Find("jnz 0x*")
        .Target()
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortCheatManager"), sizeof(void*))
        .LogSlot(TEXT("UObject::ProcessConsoleExec"))
        .Or()
        .String(TEXT("NotifyEventTriggered: Unable to find function '%s'"))
        .Start()
        .Find("call [*+0x*]")
        .Entry(TEXT("/Script/CoreUObject.Default__Object"))
        .Find("test dword ptr [*], 0x400")
        .Find("call [*+0x*]", 2)
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortCheatManager"), sizeof(void*))
        .LogSlot(TEXT("UObject::ProcessConsoleExec"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortCheatManager")));

    auto Stream = Cmd;

    if (FParse::Command(&Stream, TEXT("StartAircraft")))
    {
        if (auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode()))
            GameMode->StartAircraftPhase(false);

        return true;
    }

    return Original(Cmd, Ar, Executor);
}
