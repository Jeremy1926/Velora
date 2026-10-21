#include "pch.h"
#include "Athena/AI/AthenaAISpawner.h"
#include "FortAthenaLivingWorldManager.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Scanner.h"
#include "Detour.h"

struct _Script_FortniteGame_eventOnPawnSpawnedDynamicEvent_Parms
{
    APawn* Pawn;
    int32 RequestID;
};

void FOnPawnSpawnedDynamicEvent::ProcessMulticastDelegate(void* Parameters) const
{
    DETOUR(FOnPawnSpawnedDynamicEvent, ProcessMulticastDelegate, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .String(TEXT("UAthenaAISpawner::ProcessPendingSpawnRequests error occured during the actor spawning. Outer of the request is %s."))
        .Start()
        .Find("call [rax+0x50]; lea rcx, [*+0x38]")
        .Find("call 0x*")
        .AtCall());

    auto Parms = (_Script_FortniteGame_eventOnPawnSpawnedDynamicEvent_Parms*)Parameters;
    auto World = Parms->Pawn ? Parms->Pawn->GetWorld() : nullptr;
    auto GameState = World ? World->GetGameState() : nullptr;

    if (auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass()) : nullptr)
        LivingWorldManager->OnActorSpawned(Parms->Pawn, Parms->RequestID);

    Original(Parameters);
}

int32 UAthenaAISpawner::RequestSpawn(UFortAthenaAISpawnerDataComponentList* AISpawnerComponentList, const FTransform& SpawnTransform)
{
    static auto RequestSpawn = Anchor::Function<int32 (*)(UAthenaAISpawner*, UFortAthenaAISpawnerDataComponentList*, const FTransform&)>(TEXT("UAthenaAISpawner::RequestSpawn"))
        .Native(TEXT("/Script/FortniteGame.AthenaAISpawner:RequestSpawn"))
        .Get();

    return RequestSpawn(this, AISpawnerComponentList, SpawnTransform);
}
