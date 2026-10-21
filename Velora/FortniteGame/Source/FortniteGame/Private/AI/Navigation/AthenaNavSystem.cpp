#include "pch.h"
#include "AI/Navigation/AthenaNavSystem.h"
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
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__AthenaNavSystem")));

    if (GetNavDataSet().Num() == 0)
    {
        if (GetNavDataRegistrationQueue().Num() > 0)
            ProcessRegistrationCandidates();

        if (GetNavDataSet().Num() == 0)
        {
            static auto CanCreateInCurrentContext = THook(TEXT("UObject::CanCreateInCurrentContext"), &UObject::CanCreateInCurrentContext)
                .String(TEXT("Unable to spawn class '%s' due to client/server context."))
                .Last("call 0x*; test al, al")
                .AtCall();

            SpawnMissingNavigationData();
            ProcessRegistrationCandidates();

            static auto bRebuildOnInitialUnlock = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bRebuildOnInitialUnlock")));

            if (bRebuildOnInitialUnlock)
                bRebuildOnInitialUnlock->SetPropertyValue_InContainer(this, true);
        }
    }

    Original();
}
