#pragma once

#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineBaseTypes.h"
#include "Math/Vector.h"
#include "Misc/NetworkGuid.h"
#include "Templates/SharedPointer.h"

class AActor;
class FNetGUIDCache;
class FNetworkNotify;
class FNetworkObjectList;
class UActorChannel;
class UNetConnection;
class UWorld;
struct FNetViewer;
struct FNetworkObjectInfo;
struct FURL;

struct FActorPriority
{
    int32 Priority;

    FNetworkObjectInfo* ActorInfo;
    UActorChannel* Channel;

    struct FActorDestructionInfo* DestructionInfo;

    FActorPriority()
        : Priority(0)
        , ActorInfo(nullptr)
        , Channel(nullptr)
        , DestructionInfo(nullptr)
    {
    }

    FActorPriority(UNetConnection* InConnection, UActorChannel* InChannel, FNetworkObjectInfo* InActorInfo, const TArray<FNetViewer>& Viewers, bool bLowBandwidth);

    FActorPriority(UNetConnection* InConnection, struct FActorDestructionInfo* DestructInfo, const TArray<FNetViewer>& Viewers);
};

struct FCompareFActorPriority
{
    FORCEINLINE bool operator()(const FActorPriority& A, const FActorPriority& B) const
    {
        return B.Priority < A.Priority;
    }
};

struct FActorDestructionInfo
{
    TWeakObjectPtr<UObject> ObjOuter;
    FVector DestroyedPosition;
    FNetworkGUID NetGUID;
    FString PathName;
    FName StreamingLevelName;
};

class UNetDriver : public UObject
{
public:
    int32 GetMaxInternetClientRate() const;

    int32 GetMaxClientRate() const;

    void SetMaxClientRate(int32 InMaxClientRate);

    float GetServerTravelPause() const;

    TArray<UNetConnection*>& GetClientConnections();

    FName GetNetDriverName() const;

    float GetTime() const;

    float GetSpawnPrioritySeconds() const;

    float GetRelevantTimeout() const;

    TSharedPtr<FNetGUIDCache>& GetGuidCache();

    int32 GetNetTag() const;

    void SetNetTag(int32 InNetTag);

    TMap<FNetworkGUID, FActorDestructionInfo>& GetDestroyedStartupOrDormantActors();

    uint32 GetReplicationFrame() const;

    void SetReplicationFrame(uint32 InReplicationFrame);

    FNetworkObjectList& GetNetworkObjectList();

    bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);

    void SetWorld(UWorld* InWorld);

    void TickFlush(float DeltaSeconds);

    bool IsServer() const;

    ENetMode GetNetMode() const;

    bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const;

    static bool IsAdaptiveNetUpdateFrequencyEnabled();

    int32 ServerReplicateActors(float DeltaSeconds);

private:
    int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);

    void ServerReplicateActors_BuildConsiderList(TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime);

    int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);

    int32 ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
};
