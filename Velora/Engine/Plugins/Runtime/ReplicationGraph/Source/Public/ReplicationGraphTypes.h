#pragma once

#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Containers/UnrealString.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Misc/NetworkGuid.h"
#include "Templates/SharedPointer.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class FOutBunch;
class UClass;
class UNetConnection;
struct FNetViewer;
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

struct FRPCSendPolicyInfo
{
    uint8 bSendImmediately : 1;
};

template<typename ValueType>
struct TClassMap
{
    ValueType& GetChecked(UClass* Class)
    {
        return *Get(Class);
    }

    ValueType* Get(UClass* Class)
    {
        FObjectKey ObjKey(Class);

        if (ValueType* Ptr = Map.Find(ObjKey))
            return Ptr;

        auto Callable = *(bool (**)(void*, UClass*&, ValueType&))InitNewElement;

        if (Callable)
        {
            auto Storage = InitNewElement + 0x10;
            auto OwnedObject = *(uint8**)Storage ? *(uint8**)Storage : Storage + 0x10;
            auto GetAddress = (*(void* (***)(void*))OwnedObject)[1];
            ValueType NewValue{};

            if (Callable(GetAddress(OwnedObject), Class, NewValue))
                return &Map.Add(ObjKey, MoveTemp(NewValue));
        }

        return GetClassInfoForNewClass_r(ObjKey, Class);
    }

    alignas(16) uint8 InitNewElement[0x40];

private:
    ValueType* GetClassInfoForNewClass_r(FObjectKey OriginalObjKey, const UClass* OriginalClass)
    {
        const UClass* Class = (const UClass*)OriginalClass->GetSuperStruct();

        if (!Class)
            return nullptr;

        FObjectKey ObjKey(Class);

        if (ValueType* Ptr = Map.Find(ObjKey))
        {
            ValueType LocalData = *Ptr;

            return &Map.Add(OriginalObjKey, LocalData);
        }

        return GetClassInfoForNewClass_r(ObjKey, Class);
    }

    TMap<FObjectKey, ValueType> Map;
};

struct FFastSharedReplicationInfo
{
    uint32 LastAttemptBuildFrameNum;
    uint32 LastBunchBuildFrameNum;

    FOutBunch& GetBunch()
    {
        return *(FOutBunch*)(this + 1);
    }
};

struct FGlobalActorReplicationInfo
{
    uint16 GetReplicationPeriodFrame() const;

    uint16 GetFastPathReplicationPeriodFrame() const;

    float GetCullDistanceSquared() const;

    FVector GetWorldLocation() const;

    void SetWorldLocation(const FVector& InWorldLocation);

    float GetCullDistance() const;

    uint32& GetLastPreReplicationFrame();

    uint32 GetForceNetUpdateFrame() const;

    bool GetWantsToBeDormant() const;

    bool GetSwapRolesOnReplicate() const;

    float GetDistancePriorityScale() const;

    float GetStarvationPriorityScale() const;

    float GetAccumulatedNetPriorityBias() const;

    uint16 GetActorChannelFrameTimeout() const;

    TArray<AActor*>& GetDependentActorList();

    uint8* GetFastSharedReplicationFunc();

    FName GetFastSharedReplicationFuncName() const;

    FFastSharedReplicationInfo*& GetFastSharedReplicationInfo();

private:
    const uint8* GetSettings() const;
};

struct FGlobalActorReplicationInfoMap
{
public:
    FGlobalActorReplicationInfo& Get(AActor* const& Actor);

    FGlobalActorReplicationInfo* Find(AActor* const& Actor);

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

    FVector GetCachedPosition() const
    {
        return TGameLayout<FVector>::Load(this + 1);
    }

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

    void Emplace(const FCachedDestructInfo& Info)
    {
        memcpy(&TRuntimeSizedArray::operator[](AddUninitialized(1)), &Info, FCachedDestructInfo::GetTypeSize());
    }
};

struct FCachedDormantDestructInfo
{
    TWeakObjectPtr<ULevel> Level;
    TWeakObjectPtr<UObject> ObjOuter;
    FNetworkGUID NetGUID;
    FString PathName;
};

struct FActorRepListRefView
{
    bool ConditionalAdd(AActor* NewElement);

    TArray<AActor*> RepList;
};

struct FNewReplicatedActorInfo
{
    FNewReplicatedActorInfo(AActor* InActor);

    static FName GetStreamingLevelNameOfActor(const AActor* Actor);

    AActor* Actor;
    FName StreamingLevelName;
    UClass* Class;
};

struct FLastLocationGatherInfo
{
    static FORCEINLINE int32 GetTypeSize()
    {
        return (int32)((sizeof(UNetConnection*) + 2 * TGameLayout<FVector>::Size() + 7) & ~7);
    }

    FLastLocationGatherInfo(UNetConnection* InConnection, const FVector& InLastLocation)
        : Connection(InConnection)
    {
        TGameLayout<FVector>::Store(this + 1, InLastLocation);
        TGameLayout<FVector>::Store((uint8*)(this + 1) + TGameLayout<FVector>::Size(), InLastLocation);
    }

    FVector GetLastLocation() const
    {
        return TGameLayout<FVector>::Load(this + 1);
    }

    void SetLastLocation(const FVector& InLastLocation)
    {
        TGameLayout<FVector>::Store(this + 1, InLastLocation);
    }

    FVector GetLastOutOfRangeLocationCheck() const
    {
        return TGameLayout<FVector>::Load((const uint8*)(this + 1) + TGameLayout<FVector>::Size());
    }

    void SetLastOutOfRangeLocationCheck(const FVector& InLastOutOfRangeLocationCheck)
    {
        TGameLayout<FVector>::Store((uint8*)(this + 1) + TGameLayout<FVector>::Size(), InLastOutOfRangeLocationCheck);
    }

    UNetConnection* Connection;
};

template<>
class TArray<FLastLocationGatherInfo, FDefaultAllocator> : public TRuntimeSizedArray<FLastLocationGatherInfo>
{
public:
    FLastLocationGatherInfo* FindByKey(UNetConnection* Connection)
    {
        for (auto& GatherInfo : *this)
        {
            if (GatherInfo.Connection == Connection)
                return &GatherInfo;
        }

        return nullptr;
    }

    FLastLocationGatherInfo& Emplace(UNetConnection* InConnection, const FVector& InLastLocation)
    {
        return *new(&TRuntimeSizedArray::operator[](AddUninitialized(1))) FLastLocationGatherInfo(InConnection, InLastLocation);
    }
};

enum class EActorRepListTypeFlags : uint8
{
    Default = 0,
    FastShared = 1,
    Max,
};

template<>
class TArray<FNetViewer, TInlineAllocator<4>>
{
public:
    TArray();

    ~TArray();

    TArray(const TArray&) = delete;

    TArray& operator=(const TArray&) = delete;

    int32 Num() const;

    FNetViewer& operator[](int32 Index) const;

    TRuntimeSizedArray<FNetViewer>::TIterator begin() const;

    TRuntimeSizedArray<FNetViewer>::TIterator end() const;

    FNetViewer& Emplace(UNetConnection* InConnection, float DeltaSeconds);

private:
    uint8*& GetSecondaryData() const;

    int32& GetArrayNum() const;

    int32& GetArrayMax() const;

    uint8* GetData() const;

    alignas(16) uint8 Storage[0x200];
};

typedef TArray<FNetViewer, TInlineAllocator<4>> FNetViewerArray;

struct FReplicationGraphDestructionSettings
{
    FReplicationGraphDestructionSettings(float InDestructInfoMaxDistanceSquared, float InOutOfRangeDistanceCheckThresholdSquared)
        : DestructInfoMaxDistanceSquared(InDestructInfoMaxDistanceSquared)
        , OutOfRangeDistanceCheckThresholdSquared(InOutOfRangeDistanceCheckThresholdSquared)
    {
    }

    float DestructInfoMaxDistanceSquared;
    float OutOfRangeDistanceCheckThresholdSquared;
};

struct FPrioritizedRepList
{
    struct FItem
    {
        FItem(float InPriority, AActor* InActor, FGlobalActorReplicationInfo* InGlobal, FConnectionReplicationActorInfo* InConn)
            : Priority(InPriority)
            , Actor(InActor)
            , GlobalData(InGlobal)
            , ConnectionData(InConn)
        {
        }

        bool operator<(const FItem& Other) const
        {
            return Priority < Other.Priority;
        }

        float Priority;
        AActor* Actor;
        FGlobalActorReplicationInfo* GlobalData;
        FConnectionReplicationActorInfo* ConnectionData;
    };

    TArray<FItem> Items;
};

struct FFrameReplicationStats
{
    int32 NumReplicatedActors;
    int32 NumReplicatedCleanActors;
    int32 NumReplicatedFastPathActors;
    int32 NumConnections;
};

struct FGatheredReplicationActorLists
{
    void AddReplicationActorList(const FActorRepListRefView& List, EActorRepListTypeFlags Flags = EActorRepListTypeFlags::Default)
    {
        if (List.RepList.Num() > 0)
        {
            ReplicationLists[(uint32)Flags].Add(&List);
            CachedNum++;
        }
    }

    TArray<const FActorRepListRefView*> ReplicationLists[(uint32)EActorRepListTypeFlags::Max];
    int32 CachedNum;
};

bool IsActorValidForReplication(const AActor* In);

bool IsActorValidForReplicationGather(const AActor* In);

class UNetReplicationGraphConnection;

struct FConnectionGatherActorListParameters
{
    FConnectionGatherActorListParameters(FNetViewerArray& InViewers, UNetReplicationGraphConnection& InConnectionManager, TSet<FName>& InClientVisibleLevelNamesRef, uint32 InReplicationFrameNum, FGatheredReplicationActorLists& InOutGatheredReplicationLists);

    ~FConnectionGatherActorListParameters();

    FConnectionGatherActorListParameters(const FConnectionGatherActorListParameters&) = delete;

    FConnectionGatherActorListParameters& operator=(const FConnectionGatherActorListParameters&) = delete;

    int32 GetNumViewers() const;

    const FNetViewer& GetViewer(int32 Index) const;

    const FNetViewerArray& GetViewers() const;

    UNetReplicationGraphConnection* GetConnectionManager() const;

    uint32 GetReplicationFrameNum() const;

    FGatheredReplicationActorLists* GetOutGatheredReplicationLists() const;

    bool CheckClientVisibilityForLevel(const FName& StreamingLevelName) const;

private:
    alignas(16) uint8 Storage[0x200];
};

struct FStreamingLevelActorListCollection
{
    struct FStreamingLevelActors
    {
        FStreamingLevelActors(const FName& InStreamingLevelName)
            : StreamingLevelName(NAME_None)
        {
            TGameLayout<FName>::Store(&StreamingLevelName, InStreamingLevelName);
        }

        bool operator==(const FName& InStreamingLevelName) const
        {
            return TGameLayout<FName>::Load(&StreamingLevelName) == InStreamingLevelName;
        }

        FName StreamingLevelName;
        FActorRepListRefView ReplicationActorList;
    };

    void Gather(const FConnectionGatherActorListParameters& Params);

    void DeepCopyFrom(const FStreamingLevelActorListCollection& Source);

    TArray<FStreamingLevelActors, TInlineAllocator<4>> StreamingLevelLists;
};
