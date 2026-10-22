#include "pch.h"
#include "FortGameStateComponent_Campsite.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortGameStateComponent_Campsite::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CampsiteRuntime.FortGameStateComponent_Campsite"));

    return Class;
}

UFortGameStateComponent_Campsite* UFortGameStateComponent_Campsite::Get(const UObject* WorldContextObject)
{
    auto World = WorldContextObject && StaticClass() ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? (AActor*)World->GetGameState() : nullptr;

    return GameState ? (UFortGameStateComponent_Campsite*)GameState->FindComponentByClass(StaticClass()) : nullptr;
}

TArray<AAbandonedCampsitePlacedSpawner*>& UFortGameStateComponent_Campsite::GetPlacedCampsiteSpawnPoints()
{
    static auto PlacedCampsiteSpawnPoints = StaticClass()->FindPropertyByName(FName(TEXT("PlacedCampsiteSpawnPoints")));

    return *PlacedCampsiteSpawnPoints->ContainerPtrToValuePtr<TArray<AAbandonedCampsitePlacedSpawner*>>(this);
}
