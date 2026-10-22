#include "pch.h"
#include "FortGameStateComponent_VictoryCrowns.h"
#include "FortControllerComponent_VictoryCrowns.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortGameStateComponent_VictoryCrowns::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/VictoryCrownsRuntime.FortGameStateComponent_VictoryCrowns"));

    return Class;
}

UFortGameStateComponent_VictoryCrowns* UFortGameStateComponent_VictoryCrowns::Get(const UObject* WorldContextObject)
{
    auto World = WorldContextObject && StaticClass() ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? (AActor*)World->GetGameState() : nullptr;

    return GameState ? (UFortGameStateComponent_VictoryCrowns*)GameState->FindComponentByClass(StaticClass()) : nullptr;
}

void UFortGameStateComponent_VictoryCrowns::AddPlayerState(AFortPlayerStateAthena* PlayerStateToAdd)
{
    auto PlayerController = PlayerStateToAdd ? Cast<AFortPlayerControllerAthena>(PlayerStateToAdd->GetOwner()) : nullptr;
    auto ControllerComponent = PlayerController ? PlayerController->FindComponentByClass<UFortControllerComponent_VictoryCrowns>() : nullptr;

    if (!ControllerComponent)
        return;

    if (ControllerComponent->GetCrownInPlayerInventory())
        GetCrownBearerPlayerStates().AddUnique(PlayerStateToAdd);

    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("HandleCrownStatusChanged")));

    ControllerComponent->GetOnVictoryCrownStatusChanged().AddUnique(Delegate);
}

TArray<AFortPlayerStateAthena*>& UFortGameStateComponent_VictoryCrowns::GetCrownBearerPlayerStates()
{
    static auto CrownBearerPlayerStates = StaticClass()->FindPropertyByName(FName(TEXT("CrownBearerPlayerStates")));

    return *CrownBearerPlayerStates->ContainerPtrToValuePtr<TArray<AFortPlayerStateAthena*>>(this);
}
