#include "pch.h"
#include "AI/Navigation/AthenaNavSystem.h"
#include "AI/Navigation/AthenaNavMesh.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

struct FPendingNavGeneration
{
    TWeakObjectPtr<UAthenaNavSystem> NavSystem;
    float Time;
};

static TArray<FPendingNavGeneration> PendingNavGenerations;

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
        }
    }

    Original();

    if (!IsNavigationBeingBuilt(GetWorld()))
        return;

    if (auto AthenaNavMesh = Cast<AAthenaNavMesh>(GetMainNavData()))
    {
        AthenaNavMesh->SetUseGenerationBudgets(false);
        PendingNavGenerations.Add(FPendingNavGeneration{ this, GetWorld()->GetTimeSeconds() + 1.0f });
    }
}

void UAthenaNavSystem::TickNavGenerationTimers()
{
    for (int32 Index = PendingNavGenerations.Num() - 1; Index >= 0; Index--)
    {
        auto NavSystem = PendingNavGenerations[Index].NavSystem.Get();

        if (!NavSystem)
        {
            PendingNavGenerations.RemoveAt(Index);
            continue;
        }

        if (NavSystem->GetWorld()->GetTimeSeconds() < PendingNavGenerations[Index].Time)
            continue;

        PendingNavGenerations[Index].Time += 1.0f;

        if (IsNavigationBeingBuilt(NavSystem->GetWorld()))
            continue;

        PendingNavGenerations.RemoveAt(Index);

        if (auto AthenaNavMesh = Cast<AAthenaNavMesh>(NavSystem->GetMainNavData()))
            AthenaNavMesh->SetUseGenerationBudgets(true);
    }
}
