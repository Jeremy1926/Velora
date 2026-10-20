#pragma once

#include "Components/ActorComponent.h"
#include "Math/Vector.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class AFortPlayerState;
class UFortItemDefinition;

enum class EFortWorldMarkerType : uint8;

struct FMarkerID
{
    int32 PlayerID;
    int32 InstanceId;
};

struct FFortWorldMarkerData
{
    FMarkerID MarkerID;
    EFortWorldMarkerType MarkerType;
    FVector WorldPosition;
    FVector WorldNormal;
    UFortItemDefinition* ItemDefinition;
    TSoftClassPtr<UClass> MarkedActorClass;
    TSoftObjectPtr<AActor> MarkedActor;
    TWeakObjectPtr<AFortPlayerState> Owner;
    float CreationTime;
};

struct FFortClientMarkerRequest
{
    int32 InstanceId;
    EFortWorldMarkerType MarkerType;
    FVector WorldPosition;
    FVector WorldPositionOffset;
    FVector WorldNormal;
    AActor* MarkedActor;
};

class UAthenaMarkerComponent : public UActorComponent
{
public:
    void ClientAddMarker(AFortPlayerState* MarkerOwner, const FFortWorldMarkerData& MarkerData);

    void ClientCancelMarker(FMarkerID MarkerID);

    void ServerAddMapMarker_Implementation(FFortWorldMarkerData MarkerData);

    void ServerAddMapMarker_Implementation(FFortClientMarkerRequest MarkerRequest);

    void ServerRemoveMapMarker_Implementation(FMarkerID MarkerID);
};
