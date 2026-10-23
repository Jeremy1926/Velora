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
class FClassNetCache;
class FClassNetCacheMgr;
class FFieldNetCache;
class FNetGUIDCache;
class FNetworkNotify;
class FNetworkObjectList;
class FRepLayout;
class UActorChannel;
class UFunction;
class ULevel;
class UNetConnection;
class UReplicationDriver;
class UWorld;
struct FFrame;
struct FNetViewer;
struct FNetworkObjectInfo;
struct FOutParmRec;
struct FURL;

enum class EProcessRemoteFunctionFlags : uint8
{
    None = 0,
    ReplicatedActor = 1 << 0,
};

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
public:
    static int32 GetTypeSize();

    TWeakObjectPtr<UObject>& GetObjOuter();

    FVector GetDestroyedPosition();

    FNetworkGUID& GetNetGUID();

    FString& GetPathName();

    FName GetStreamingLevelName();

    TWeakObjectPtr<ULevel>& GetLevel();

    EChannelCloseReason& GetReason();

    bool& GetIgnoreDistanceCulling();

private:
    template<typename VectorType>
    struct TFields
    {
        TWeakObjectPtr<UObject> ObjOuter;
        VectorType DestroyedPosition;
        FNetworkGUID NetGUID;
        FString PathName;
        alignas(FName) uint8 StreamingLevelName[sizeof(FName)];
    };

    static int32 GetLevelSize();

    uint8* GetFields();
};

template<>
class TMap<FNetworkGUID, FActorDestructionInfo>
{
public:
    FORCEINLINE int32 Num() const
    {
        return Elements.Num() - NumFreeIndices;
    }

    FActorDestructionInfo& FindChecked(const FNetworkGUID& Key)
    {
        const int32 ValueOffset = (int32)sizeof(void*);
        const int32 ElementSize = ValueOffset + FActorDestructionInfo::GetTypeSize() + (int32)sizeof(FSetElementId) + (int32)sizeof(int32);
        FSetElementId ElementId = Hash.GetAllocation()[GetTypeHash(Key) & (HashSize - 1)];

        while (!(*(FNetworkGUID*)(Elements.GetData() + ElementId * ElementSize) == Key))
        {
            ElementId = *(FSetElementId*)(Elements.GetData() + ElementId * ElementSize + ValueOffset + FActorDestructionInfo::GetTypeSize());
        }

        return *(FActorDestructionInfo*)(Elements.GetData() + ElementId * ElementSize + ValueOffset);
    }

private:
    TArray<uint8> Elements;
    TBitArray<> AllocationFlags;
    int32 FirstFreeIndex;
    int32 NumFreeIndices;
    TInlineAllocator<1>::ForElementType<FSetElementId> Hash;
    int32 HashSize;
};

class UNetDriver : public UObject
{
public:
    enum ERemoteFunctionSendPolicy
    {
        Default,
        ForceSend,
        ForceQueue,
    };

    int32 GetMaxInternetClientRate() const;

    int32 GetMaxClientRate() const;

    int32 GetNetServerMaxTickRate() const;

    void SetNetServerMaxTickRate(int32 InNetServerMaxTickRate);

    void SetMaxClientRate(int32 InMaxClientRate);

    float GetServerTravelPause() const;

    TArray<UNetConnection*>& GetClientConnections();

    UReplicationDriver* GetReplicationDriver() const;

    void SetReplicationDriver(UReplicationDriver* NewReplicationDriver);

    FName GetNetDriverName() const;

    float GetTime() const;

    float GetSpawnPrioritySeconds() const;

    float GetRelevantTimeout() const;

    TSharedPtr<FNetGUIDCache>& GetGuidCache();

    TSharedPtr<FClassNetCacheMgr>& GetNetCache();

    TSharedPtr<FRepLayout, ESPMode::ThreadSafe> GetFunctionRepLayout(UFunction* Function);

    void ProcessRemoteFunctionForChannelPrivate(UActorChannel* Ch, const FClassNetCache* ClassCache, const FFieldNetCache* FieldCache, UObject* TargetObj, UNetConnection* Connection, UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack, const bool bIsServer, const ERemoteFunctionSendPolicy SendPolicy, EProcessRemoteFunctionFlags& RemoteFunctionFlags);

    int32 GetNetTag() const;

    void SetNetTag(int32 InNetTag);

    TMap<FNetworkGUID, FActorDestructionInfo>& GetDestroyedStartupOrDormantActors();

    uint32 GetReplicationFrame() const;

    void SetReplicationFrame(uint32 InReplicationFrame);

    FNetworkObjectList& GetNetworkObjectList();

    bool InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error);

    void SetWorld(UWorld* InWorld);

    void TickFlush(float DeltaSeconds);

    void FlushActorDormancy(AActor* Actor, bool bWasDormInitial);

    bool IsServer() const;

    ENetMode GetNetMode() const;

    bool IsLevelInitializedForActor(const AActor* InActor, const UNetConnection* InConnection) const;

    int64 SendDestructionInfo(UNetConnection* Connection, FActorDestructionInfo* DestructionInfo);

    static bool IsAdaptiveNetUpdateFrequencyEnabled();

    int32 ServerReplicateActors(float DeltaSeconds);

private:
    int32 ServerReplicateActors_PrepConnections(const float DeltaSeconds);

    void ServerReplicateActors_BuildConsiderList(TArray<FNetworkObjectInfo*>& OutConsiderList, const float ServerTickTime);

    int32 ServerReplicateActors_PrioritizeActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, const TArray<FNetworkObjectInfo*> ConsiderList, const bool bCPUSaturated, FActorPriority*& OutPriorityList, FActorPriority**& OutPriorityActors);

    int32 ServerReplicateActors_ProcessPrioritizedActors(UNetConnection* Connection, const TArray<FNetViewer>& ConnectionViewers, FActorPriority** PriorityActors, const int32 FinalSortedCount, int32& OutUpdated);
};
