#include "pch.h"
#include "AI/Navigation/AthenaNavSystem.h"
#include "NavigationData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

void UAthenaNavSystem::ReleaseInitialBuildingLock()
{
    DETOUR(UAthenaNavSystem, ReleaseInitialBuildingLock, Anchor::Hook()
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("call 0x*", 2)
        .Target()
        .Find("call [rax+0x*]", 2)
        .Or()
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Is("call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]", 2)
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__AthenaNavSystem"), [](const ::Anchor::FCode& Entry) { return Entry.Has("mov rbx, rcx; sub al, 0x2; test al, 0xfb; jz 0x*; test byte ptr [rcx+0x*], 0x2"); })
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__AthenaNavSystem"), [](const ::Anchor::FCode& Entry) { return Entry.Has("mov al, [rcx+0x*]; *; *; test al, 0xfb; jz 0x*; test byte ptr [rcx+0x*], 0x2"); })
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__AthenaNavSystem")));

    if (GetNavDataSet().Num() == 0)
    {
        if (GetNavDataRegistrationQueue().Num() > 0)
            ProcessRegistrationCandidates();

        if (GetNavDataSet().Num() == 0)
        {
            UObject::SetServerContext();
            UNavigationSystemV1::SpawnMissingNavigationData();
            ProcessRegistrationCandidates();
        }
    }

    static auto bRebuildOnInitialUnlock = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bRebuildOnInitialUnlock")));

    for (auto NavData : GetNavDataSet())
    {
        if (bRebuildOnInitialUnlock && NavData && !(NavData->GetFlags() & RF_WasLoaded))
        {
            bRebuildOnInitialUnlock->SetPropertyValue_InContainer(this, true);
            break;
        }
    }

    Original();
}

void UAthenaNavSystem::SpawnMissingNavigationData()
{
    DETOUR(UAthenaNavSystem, SpawnMissingNavigationData, Anchor::Hook()
        .String(TEXT("Auto generating navigation bounds: %s"))
        .Find("call [rax+0x*]; mov rax, [r*]; mov rcx, r*; call [rax+0x*]; cmp qword ptr [r*+0x*], 0x0")
        .Or()
        .String(TEXT("Auto generating navigation bounds: %s"))
        .Find("call [rax+0x*]; mov rcx, r*; mov rax, [r*]; call [rax+0x*]; cmp qword ptr [r*+0x*], 0x0")
        .Or()
        .String(TEXT("Auto generating navigation bounds: %s"))
        .Find("mov rcx, *; call 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .InVTable(TEXT("/Script/FortniteGame.Default__AthenaNavSystem")));

    UObject::SetServerContext();

    Original();
}
