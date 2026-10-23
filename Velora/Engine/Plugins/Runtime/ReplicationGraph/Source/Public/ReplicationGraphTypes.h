#pragma once

#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Misc/NetworkGuid.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class UActorChannel;
class ULevel;
class UObject;
struct FActorDestructionInfo;

template<typename NameType>
struct TClassReplicationInfoLayout
{
    float DistancePriorityScale;
    float StarvationPriorityScale;
    float AccumulatedNetPriorityBias;
    uint16 ReplicationPeriodFrame;
    uint16 FastPath_ReplicationPeriodFrame;
    uint16 ActorChannelFrameTimeout;
    alignas(16) uint8 FastSharedReplicationFunc[0x40];
    NameType FastSharedReplicationFuncName;
    float CullDistance;
    float CullDistanceSquared;
};

template<typename VectorType>
struct TGlobalActorReplicationInfoLayout
{
    uint32 LastPreReplicationFrame;
    uint32 ForceNetUpdateFrame;
    VectorType WorldLocation;
    bool bWantsToBeDormant;
    bool bSwapRolesOnReplicate;
    alignas(16) uint8 Settings[0x10];
};

struct FGlobalActorReplicationInfo
{
    uint16 GetReplicationPeriodFrame() const;

    uint16 GetFastPathReplicationPeriodFrame() const;

    float GetCullDistanceSquared() const;

private:
    const uint8* GetSettings() const;
};

struct FGlobalActorReplicationInfoMap
{
public:
    FGlobalActorReplicationInfo& Get(AActor* const& Actor);

    void ResetActorMap();

private:
    TMap<AActor*, FGlobalActorReplicationInfo*> ActorMap;
};

struct FConnectionReplicationActorInfo
{
    FConnectionReplicationActorInfo(const FGlobalActorReplicationInfo& GlobalInfo)
        : bDormantOnConnection(0)
        , bTearOff(0)
        , bGridSpatilization_AlreadyDormant(0)
    {
        ReplicationPeriodFrame = GlobalInfo.GetReplicationPeriodFrame();
        FastPath_ReplicationPeriodFrame = GlobalInfo.GetFastPathReplicationPeriodFrame();
        SetCullDistanceSquared(GlobalInfo.GetCullDistanceSquared());
    }

    void SetCullDistanceSquared(float InCullDistanceSquared)
    {
        CullDistanceSquared = InCullDistanceSquared;
        CullDistance = FMath::Sqrt(CullDistanceSquared);
    }

    UActorChannel* Channel = nullptr;
    float CullDistance = 0.f;
    float CullDistanceSquared = 0.f;
    uint32 NextReplicationFrameNum = 0;
    uint32 LastRepFrameNum = 0;
    uint32 FastPath_NextReplicationFrameNum = 0;
    uint32 FastPath_LastRepFrameNum = 0;
    uint16 ReplicationPeriodFrame = 1;
    uint16 FastPath_ReplicationPeriodFrame = 1;
    uint32 ActorChannelCloseFrameNum = 0;
    uint8 bDormantOnConnection : 1;
    uint8 bTearOff : 1;
    uint8 bGridSpatilization_AlreadyDormant : 1;
};

struct FPerConnectionActorInfoMap
{
    FConnectionReplicationActorInfo& FindOrAdd(AActor* const& Actor);

    void AddChannel(AActor* const& Actor, UActorChannel* Channel);

    void RemoveChannel(UActorChannel* Channel);

    TMap<AActor*, TSharedPtr<FConnectionReplicationActorInfo, ESPMode::ThreadSafe>> ActorMap;
    TMap<UActorChannel*, TSharedPtr<FConnectionReplicationActorInfo, ESPMode::ThreadSafe>> ChannelMap;
    FGlobalActorReplicationInfoMap* GlobalMap;
};

struct FCachedDestructInfo
{
    static FORCEINLINE int32 GetTypeSize()
    {
        return (int32)((sizeof(FActorDestructionInfo*) + TGameLayout<FVector>::Size() + 7) & ~7);
    }

    FCachedDestructInfo(FActorDestructionInfo* InDestructionInfo);

    FActorDestructionInfo* DestructionInfo;
};

template<>
class TArray<FCachedDestructInfo, FDefaultAllocator> : public TRuntimeSizedArray<FCachedDestructInfo>
{
public:
    void Emplace(FActorDestructionInfo* DestructionInfo)
    {
        new(&TRuntimeSizedArray::operator[](AddUninitialized(1))) FCachedDestructInfo(DestructionInfo);
    }
};

struct FCachedDormantDestructInfo
{
    TWeakObjectPtr<ULevel> Level;
    TWeakObjectPtr<UObject> ObjOuter;
    FNetworkGUID NetGUID;
    FString PathName;
};
