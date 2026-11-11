#include "pch.h"
#include "FortGameStateComponent_BlastBerryManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortGameStateComponent_BlastBerryManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/BlastBerryRuntime.FortGameStateComponent_BlastBerryManager"));

    return Class;
}

UFortGameStateComponent_BlastBerryManager* UFortGameStateComponent_BlastBerryManager::Get(const UObject* WorldContextObject)
{
    auto World = WorldContextObject && StaticClass() ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? (AActor*)World->GetGameState() : nullptr;

    return GameState ? (UFortGameStateComponent_BlastBerryManager*)GameState->FindComponentByClass(StaticClass()) : nullptr;
}

void UFortGameStateComponent_BlastBerryManager::HandleTeamRemovedFromGame(uint8 TeamId)
{
    static auto HandleTeamRemovedFromGame = Anchor::Function<void (*)(UFortGameStateComponent_BlastBerryManager*, uint8)>(TEXT("UFortGameStateComponent_BlastBerryManager::HandleTeamRemovedFromGame"))
        .String("UFortGameStateComponent_BlastBerryManager::HandleTeamRemovedFromGame")
        .Start()
        .Get();

    HandleTeamRemovedFromGame(this, TeamId);
}
