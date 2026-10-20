#include "pch.h"
#include "Athena/AthenaMarkerComponent.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

struct AthenaMarkerComponent_eventClientAddMarker_Parms
{
    AFortPlayerState* MarkerOwner;
    FFortWorldMarkerData MarkerData;
};

struct AthenaMarkerComponent_eventClientCancelMarker_Parms
{
    FMarkerID MarkerID;
};

static auto ServerAddMapMarker = []
{
    auto ServerAddMapMarker = Scanner::Exec(TEXT("/Script/FortniteGame.AthenaMarkerComponent:ServerAddMapMarker"));
    auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__AthenaMarkerComponent"));

    return ServerAddMapMarker.Get() ? DetourVTable(TEXT("UAthenaMarkerComponent::ServerAddMapMarker_Implementation"), *(void***)DefaultObject, ServerAddMapMarker.Find("call [*+0x*]", 2).Slot(TEXT("UAthenaMarkerComponent::ServerAddMapMarker_Implementation")), &UAthenaMarkerComponent::ServerAddMapMarker_Implementation) : nullptr;
}();

static auto ServerRemoveMapMarker = []
{
    auto ServerRemoveMapMarker = Scanner::Exec(TEXT("/Script/FortniteGame.AthenaMarkerComponent:ServerRemoveMapMarker"));
    auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__AthenaMarkerComponent"));

    return ServerRemoveMapMarker.Get() ? DetourVTable(TEXT("UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation"), *(void***)DefaultObject, ServerRemoveMapMarker.Find("call [*+0x*]", 2).Slot(TEXT("UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation")), &UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation) : nullptr;
}();

void UAthenaMarkerComponent::ClientAddMarker(AFortPlayerState* MarkerOwner, const FFortWorldMarkerData& MarkerData)
{
    static auto ClientAddMarker = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientAddMarker"));

    AthenaMarkerComponent_eventClientAddMarker_Parms Parms{ MarkerOwner, MarkerData };

    CallRemoteFunction(ClientAddMarker, &Parms, nullptr, nullptr);
}

void UAthenaMarkerComponent::ClientCancelMarker(FMarkerID MarkerID)
{
    static auto ClientCancelMarker = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientCancelMarker"));

    AthenaMarkerComponent_eventClientCancelMarker_Parms Parms{ MarkerID };

    CallRemoteFunction(ClientCancelMarker, &Parms, nullptr, nullptr);
}

void UAthenaMarkerComponent::ServerAddMapMarker_Implementation(FFortWorldMarkerData MarkerData)
{
    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetOwner());
    auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;

    if (!PlayerState)
        return;

    for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
    {
        auto OtherPlayerController = Cast<AFortPlayerControllerAthena>(OtherPlayerState->GetOwner());

        if (OtherPlayerController && OtherPlayerController != PlayerController && OtherPlayerController->GetSquadId() == PlayerState->GetSquadId())
        {
            if (auto MarkerComponent = OtherPlayerController->GetMarkerComponent())
                MarkerComponent->ClientAddMarker(PlayerState, MarkerData);
        }
    }
}

void UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation(FMarkerID MarkerID)
{
    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetOwner());
    auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;

    if (!PlayerState)
        return;

    for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
    {
        auto OtherPlayerController = Cast<AFortPlayerControllerAthena>(OtherPlayerState->GetOwner());

        if (OtherPlayerController && OtherPlayerController != PlayerController && OtherPlayerController->GetSquadId() == PlayerState->GetSquadId())
        {
            if (auto MarkerComponent = OtherPlayerController->GetMarkerComponent())
                MarkerComponent->ClientCancelMarker(MarkerID);
        }
    }
}
