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
    static auto ClientAddMarker = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientAddMarker"));
    static auto MarkerDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldMarkerData"));
    static auto MarkerRequestStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortClientMarkerRequest"));
    static auto Constructor = ClientAddMarker ? Scanner::Function<void (*)(FFortWorldMarkerData*)>(Scanner::Exec(TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientAddMarker")).Call().Get(TEXT("FFortWorldMarkerData::FFortWorldMarkerData"))) : nullptr;
    static auto Destructor = ClientAddMarker ? Scanner::Function<void (*)(FFortWorldMarkerData*)>([]
    {
        auto ClientAddMarker_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientAddMarker")).Find("call [*+0x*]").Slot(TEXT("UAthenaMarkerComponent::ClientAddMarker_Implementation"));
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__AthenaMarkerComponent"));

        return Scanner::At(Scanner::Virtual<const void*>(DefaultObject, ClientAddMarker_Implementation)).Find("jmp 0x*").Target(TEXT("FFortWorldMarkerData::~FFortWorldMarkerData"));
    }()) : nullptr;
    static auto BuildMarkerDataForActor = Scanner::Function<bool (*)(UAthenaMarkerComponent*, AActor*, FFortWorldMarkerData*)>(Scanner::CallTo(Scanner::String(TEXT("FortMarkableActorInterface")).Start().Get()).Start().Get(TEXT("UAthenaMarkerComponent::BuildMarkerDataForActor")));
    static auto MarkerID = MarkerDataStruct->FindPropertyByName(FName(TEXT("MarkerID")));
    static auto Owner = MarkerDataStruct->FindPropertyByName(FName(TEXT("Owner")));
    static auto MarkerType = MarkerDataStruct->FindPropertyByName(FName(TEXT("MarkerType")));
    static auto WorldPosition = []
    {
        auto WorldPosition = MarkerDataStruct->FindPropertyByName(FName(TEXT("WorldPosition")));

        return WorldPosition ? WorldPosition : MarkerDataStruct->FindPropertyByName(FName(TEXT("BasePosition")));
    }();
    static auto WorldPositionOffset = []
    {
        auto WorldPositionOffset = MarkerDataStruct->FindPropertyByName(FName(TEXT("WorldPositionOffset")));

        return WorldPositionOffset ? WorldPositionOffset : MarkerDataStruct->FindPropertyByName(FName(TEXT("BasePositionOffset")));
    }();
    static auto WorldNormal = MarkerDataStruct->FindPropertyByName(FName(TEXT("WorldNormal")));
    static auto RequestIncludeSquad = (UBoolProperty*)MarkerRequestStruct->FindPropertyByName(FName(TEXT("bIncludeSquad")));
    static auto RequestUseHoveredMarkerDetail = (UBoolProperty*)MarkerRequestStruct->FindPropertyByName(FName(TEXT("bUseHoveredMarkerDetail")));
    static auto bIncludeSquad = (UBoolProperty*)MarkerDataStruct->FindPropertyByName(FName(TEXT("bIncludeSquad")));
    static auto bUseHoveredMarkerDetail = (UBoolProperty*)MarkerDataStruct->FindPropertyByName(FName(TEXT("bUseHoveredMarkerDetail")));

    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetOwner());
    auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;

    if (!PlayerState)
        return;

    static auto PlayerId = PlayerState->GetClass()->FindPropertyByName(FName(TEXT("PlayerId")));

    TArray<uint8> Buffer;
    Buffer.AddUninitialized(MarkerDataStruct->GetPropertiesSize());

    auto MarkerData = (FFortWorldMarkerData*)Buffer.GetData();

    if (ClientAddMarker)
        Constructor(MarkerData);
    else
        MarkerDataStruct->InitializeStruct(MarkerData);

    if (MarkerRequest.MarkedActor)
        BuildMarkerDataForActor(this, MarkerRequest.MarkedActor, MarkerData);

    *MarkerID->ContainerPtrToValuePtr<FMarkerID>(MarkerData) = FMarkerID{ *PlayerId->ContainerPtrToValuePtr<int32>(PlayerState), MarkerRequest.InstanceId };
    *Owner->ContainerPtrToValuePtr<AFortPlayerState*>(MarkerData) = PlayerState;
    *MarkerType->ContainerPtrToValuePtr<EFortWorldMarkerType>(MarkerData) = MarkerRequest.MarkerType;
    *WorldPosition->ContainerPtrToValuePtr<FVector>(MarkerData) = MarkerRequest.WorldPosition;
    *WorldPositionOffset->ContainerPtrToValuePtr<FVector>(MarkerData) = MarkerRequest.WorldPositionOffset;
    *WorldNormal->ContainerPtrToValuePtr<FVector>(MarkerData) = MarkerRequest.WorldNormal;

    if (RequestIncludeSquad)
        bIncludeSquad->SetPropertyValue_InContainer(MarkerData, RequestIncludeSquad->GetPropertyValue_InContainer(&MarkerRequest));

    if (RequestUseHoveredMarkerDetail)
        bUseHoveredMarkerDetail->SetPropertyValue_InContainer(MarkerData, RequestUseHoveredMarkerDetail->GetPropertyValue_InContainer(&MarkerRequest));

    for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
    {
        auto OtherPlayerController = Cast<AFortPlayerControllerAthena>(OtherPlayerState->GetOwner());

        if (OtherPlayerController && OtherPlayerController != PlayerController && OtherPlayerController->GetSquadId() == PlayerState->GetSquadId())
        {
            if (auto MarkerComponent = OtherPlayerController->GetMarkerComponent())
            {
                if (ClientAddMarker)
                {
                    MarkerComponent->ClientAddMarker(PlayerState, *MarkerData);
                }
                else if (bIncludeSquad->GetPropertyValue_InContainer(MarkerData))
                {
                    auto& MarkerStream = MarkerComponent->GetMarkerStream();
                    MarkerStream.MarkItemDirty((FFastArraySerializerItem&)MarkerStream.AddOrGetMarker(*MarkerData));
                }
            }
        }
    }

    if (ClientAddMarker)
        Destructor(MarkerData);
    else
        MarkerDataStruct->DestroyStruct(MarkerData);
}

void UAthenaMarkerComponent::ServerRemoveMapMarker_Implementation(FMarkerID MarkerID)
{
    static auto ClientCancelMarker = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaMarkerComponent:ClientCancelMarker"));

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
            {
                if (ClientCancelMarker)
                    MarkerComponent->ClientCancelMarker(MarkerID);
                else
                    MarkerComponent->GetMarkerStream().RemoveMarker(MarkerID);
            }
        }
    }
}

FFortWorldMarkerContainer& UAthenaMarkerComponent::GetMarkerStream()
{
    static auto MarkerStream = GetClass()->FindPropertyByName(FName(TEXT("MarkerStream")));

    return *MarkerStream->ContainerPtrToValuePtr<FFortWorldMarkerContainer>(this);
}

UScriptStruct* FFortWorldMarkerContainer::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldMarkerContainer"));

    return Struct;
}

FFortWorldMarkerData& FFortWorldMarkerContainer::AddOrGetMarker(const FFortWorldMarkerData& MarkerData)
{
    static auto MarkerDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldMarkerData"));
    static auto MarkerID = MarkerDataStruct->FindPropertyByName(FName(TEXT("MarkerID")));
    static auto Owner = MarkerDataStruct->FindPropertyByName(FName(TEXT("Owner")));
    static auto TypeSize = MarkerDataStruct->GetPropertiesSize();

    auto& Markers = GetMarkers();
    auto& NewMarkerID = *MarkerID->ContainerPtrToValuePtr<FMarkerID>(&MarkerData);

    for (int32 Index = 0; Index < Markers.Num(); Index++)
    {
        auto Marker = (FFortWorldMarkerData*)((uint8*)Markers.GetData() + Index * TypeSize);
        auto& ExistingMarkerID = *MarkerID->ContainerPtrToValuePtr<FMarkerID>(Marker);

        if (*Owner->ContainerPtrToValuePtr<AFortPlayerState*>(Marker) == *Owner->ContainerPtrToValuePtr<AFortPlayerState*>(&MarkerData) && ExistingMarkerID.PlayerID == NewMarkerID.PlayerID && ExistingMarkerID.InstanceId == NewMarkerID.InstanceId)
            return *Marker;
    }

    auto Index = Markers.Add(1, TypeSize);
    auto Marker = (FFortWorldMarkerData*)((uint8*)Markers.GetData() + Index * TypeSize);

    MarkerDataStruct->InitializeStruct(Marker);
    MarkerDataStruct->CopyScriptStruct(Marker, &MarkerData);

    return *Marker;
}

bool FFortWorldMarkerContainer::RemoveMarker(const FMarkerID& MarkerID)
{
    static auto MarkerDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldMarkerData"));
    static auto MarkerIDProperty = MarkerDataStruct->FindPropertyByName(FName(TEXT("MarkerID")));
    static auto TypeSize = MarkerDataStruct->GetPropertiesSize();

    auto& Markers = GetMarkers();
    int32 NumRemoved = 0;

    for (int32 Index = 0; Index < Markers.Num();)
    {
        auto Marker = (FFortWorldMarkerData*)((uint8*)Markers.GetData() + Index * TypeSize);
        auto& ExistingMarkerID = *MarkerIDProperty->ContainerPtrToValuePtr<FMarkerID>(Marker);

        if (ExistingMarkerID.PlayerID == MarkerID.PlayerID && ExistingMarkerID.InstanceId == MarkerID.InstanceId)
        {
            MarkerDataStruct->DestroyStruct(Marker);
            Markers.Remove(Index, 1, TypeSize);
            NumRemoved++;
        }
        else
        {
            Index++;
        }
    }

    MarkArrayDirty();

    return NumRemoved > 0;
}

FScriptArray& FFortWorldMarkerContainer::GetMarkers()
{
    static auto Markers = StaticStruct()->FindPropertyByName(FName(TEXT("Markers")));

    return *Markers->ContainerPtrToValuePtr<FScriptArray>(this);
}
