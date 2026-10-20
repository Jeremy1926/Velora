#include "pch.h"
#include "Athena/AthenaMarkerComponent.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
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

    if (!ServerAddMapMarker.Get())
        return false;

    auto Implementation = ServerAddMapMarker.Find("call [*+0x*]", 2);
    auto Slot = (Implementation.Get() ? Implementation : ServerAddMapMarker.Find("call [*+0x*]")).Slot(TEXT("UAthenaMarkerComponent::ServerAddMapMarker_Implementation"));

    if (((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaMarkerComponent:ServerAddMapMarker")))->FindPropertyByName(FName(TEXT("MarkerRequest"))))
        DetourVTable(TEXT("UAthenaMarkerComponent::ServerAddMapMarker_Implementation"), *(void***)DefaultObject, Slot, static_cast<void (UAthenaMarkerComponent::*)(FFortClientMarkerRequest)>(&UAthenaMarkerComponent::ServerAddMapMarker_Implementation));
    else
        DetourVTable(TEXT("UAthenaMarkerComponent::ServerAddMapMarker_Implementation"), *(void***)DefaultObject, Slot, static_cast<void (UAthenaMarkerComponent::*)(FFortWorldMarkerData)>(&UAthenaMarkerComponent::ServerAddMapMarker_Implementation));

    return true;
}();

static auto ServerRemoveMapMarker = []
{
    auto ServerRemoveMapMarker = Scanner::Exec(TEXT("/Script/FortniteGame.AthenaMarkerComponent:ServerRemoveMapMarker"));
    auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__AthenaMarkerComponent"));

    auto Implementation = ServerRemoveMapMarker.Find("call [*+0x*]", 2);

    return ServerRemoveMapMarker.Get() ? DetourVTable(TEXT("UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation"), *(void***)DefaultObject, (Implementation.Get() ? Implementation : ServerRemoveMapMarker.Find("call [*+0x*]")).Slot(TEXT("UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation")), &UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation) : nullptr;
}();

void UAthenaMarkerComponent::ClientAddMarker(AFortPlayerState* MarkerOwner, const FFortWorldMarkerData& MarkerData)
{
    static auto ClientAddMarker = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientAddMarker"));
    static auto bMarkerOwner = ClientAddMarker->FindPropertyByName(FName(TEXT("MarkerOwner"))) != nullptr;

    if (!bMarkerOwner)
    {
        CallRemoteFunction(ClientAddMarker, (void*)&MarkerData, nullptr, nullptr);
        return;
    }

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

void UAthenaMarkerComponent::ServerAddMapMarker_Implementation(FFortClientMarkerRequest MarkerRequest)
{
    static auto MarkerDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldMarkerData"));
    static auto Constructor = Scanner::Function<void (*)(FFortWorldMarkerData*)>(Scanner::Exec(TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientAddMarker")).Call().Get(TEXT("FFortWorldMarkerData::FFortWorldMarkerData")));
    static auto Destructor = Scanner::Function<void (*)(FFortWorldMarkerData*)>([]
    {
        auto ClientAddMarker_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientAddMarker")).Find("call [*+0x*]").Slot(TEXT("UAthenaMarkerComponent::ClientAddMarker_Implementation"));
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__AthenaMarkerComponent"));

        return Scanner::At(Scanner::Virtual<const void*>(DefaultObject, ClientAddMarker_Implementation)).Find("jmp 0x*").Target(TEXT("FFortWorldMarkerData::~FFortWorldMarkerData"));
    }());
    static auto BuildMarkerDataForActor = Scanner::Function<bool (*)(UAthenaMarkerComponent*, AActor*, FFortWorldMarkerData*)>(Scanner::CallTo(Scanner::String(TEXT("FortMarkableActorInterface")).Start().Get()).Start().Get(TEXT("UAthenaMarkerComponent::BuildMarkerDataForActor")));
    static auto MarkerID = MarkerDataStruct->FindPropertyByName(FName(TEXT("MarkerID")));
    static auto Owner = MarkerDataStruct->FindPropertyByName(FName(TEXT("Owner")));
    static auto MarkerType = MarkerDataStruct->FindPropertyByName(FName(TEXT("MarkerType")));
    static auto WorldPosition = MarkerDataStruct->FindPropertyByName(FName(TEXT("WorldPosition")));
    static auto WorldPositionOffset = MarkerDataStruct->FindPropertyByName(FName(TEXT("WorldPositionOffset")));
    static auto WorldNormal = MarkerDataStruct->FindPropertyByName(FName(TEXT("WorldNormal")));

    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetOwner());
    auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;

    if (!PlayerState)
        return;

    static auto PlayerId = PlayerState->GetClass()->FindPropertyByName(FName(TEXT("PlayerId")));

    TArray<uint8> Buffer;
    Buffer.AddUninitialized(MarkerDataStruct->GetPropertiesSize());

    auto MarkerData = (FFortWorldMarkerData*)Buffer.GetData();

    Constructor(MarkerData);

    if (MarkerRequest.MarkedActor)
        BuildMarkerDataForActor(this, MarkerRequest.MarkedActor, MarkerData);

    *MarkerID->ContainerPtrToValuePtr<FMarkerID>(MarkerData) = FMarkerID{ *PlayerId->ContainerPtrToValuePtr<int32>(PlayerState), MarkerRequest.InstanceId };
    *Owner->ContainerPtrToValuePtr<AFortPlayerState*>(MarkerData) = PlayerState;
    *MarkerType->ContainerPtrToValuePtr<EFortWorldMarkerType>(MarkerData) = MarkerRequest.MarkerType;
    *WorldPosition->ContainerPtrToValuePtr<FVector>(MarkerData) = MarkerRequest.WorldPosition;
    *WorldPositionOffset->ContainerPtrToValuePtr<FVector>(MarkerData) = MarkerRequest.WorldPositionOffset;
    *WorldNormal->ContainerPtrToValuePtr<FVector>(MarkerData) = MarkerRequest.WorldNormal;

    for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
    {
        auto OtherPlayerController = Cast<AFortPlayerControllerAthena>(OtherPlayerState->GetOwner());

        if (OtherPlayerController && OtherPlayerController != PlayerController && OtherPlayerController->GetSquadId() == PlayerState->GetSquadId())
        {
            if (auto MarkerComponent = OtherPlayerController->GetMarkerComponent())
                MarkerComponent->ClientAddMarker(PlayerState, *MarkerData);
        }
    }

    Destructor(MarkerData);
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
