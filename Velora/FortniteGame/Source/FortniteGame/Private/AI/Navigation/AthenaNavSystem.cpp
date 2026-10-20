#include "pch.h"
#include "AI/Navigation/AthenaNavSystem.h"
#include "Athena/AI/AthenaAISystem.h"
#include "Athena/FortGameModeAthena.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

void UAthenaNavSystem::RemoveNavigationLocks()
{
    DETOUR(UAthenaNavSystem, RemoveNavigationLocks, Anchor::Hook()
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("call 0x*", 2)
        .Target()
        .Inline());

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
        }
    }

    auto World = GetWorld();
    auto AISystem = World ? Cast<UAthenaAISystem>(World->GetAISystem()) : nullptr;
    auto GameMode = World ? Cast<AFortGameModeAthena>(World->GetAuthGameMode()) : nullptr;

    if (AISystem && GameMode && GameMode->GetServerBotManager())
        AISystem->GetPlayerBotManager() = GameMode->GetServerBotManager();

    Original();
}
