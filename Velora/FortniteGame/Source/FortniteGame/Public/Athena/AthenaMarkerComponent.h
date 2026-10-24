#pragma once

#include "Components/ActorComponent.h"
#include "Containers/ScriptArray.h"
#include "Engine/NetSerialization.h"
#include "Math/Vector.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class AActor;
class AFortPlayerState;
class UFortItemDefinition;
class UScriptStruct;

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

struct FFortWorldMarkerContainer : public FFastArraySerializer
{
    static UScriptStruct* StaticStruct();

    FFortWorldMarkerData& AddOrGetMarker(const FFortWorldMarkerData& MarkerData);

    bool RemoveMarker(const FMarkerID& MarkerID);

    FScriptArray& GetMarkers();
};

struct FFortClientMarkerRequest
{
    static UScriptStruct* StaticStruct();

    int32 GetInstanceId() const;

    EFortWorldMarkerType GetMarkerType() const;

    FVector GetWorldPosition() const;

    FVector GetWorldPositionOffset() const;

    FVector GetWorldNormal() const;

    AActor* GetMarkedActor() const;
};

class UAthenaMarkerComponent : public UActorComponent
{
public:
    void ClientAddMarker(AFortPlayerState* MarkerOwner, const FFortWorldMarkerData& MarkerData);

    void ClientCancelMarker(FMarkerID MarkerID);

    void ServerAddMapMarker_Implementation(FFortWorldMarkerData MarkerData);

    void ServerAddMapMarker_Implementation(const FFortClientMarkerRequest& MarkerRequest);

    void ServerRemoveMapMarker_Implementation(FMarkerID MarkerID);

    FFortWorldMarkerContainer& GetMarkerStream();
};
