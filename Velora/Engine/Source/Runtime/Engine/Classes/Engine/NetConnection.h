#pragma once

#include "Engine/Player.h"
#include "Engine/Channel.h"
#include "Containers/Array.h"
#include "Containers/Set.h"
#include "Containers/Map.h"
#include "Misc/NetworkGuid.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class ULevel;
class UActorChannel;
class UChannel;
class UChildConnection;
class UNetDriver;
class UPackageMap;
class UReplicationConnectionDriver;

enum class EChannelCreateFlags : uint32
{
    None = (1 << 0),
    OpenedLocally = (1 << 1),
};

enum EConnectionState
{
    USOCK_Invalid = 0,
    USOCK_Closed = 1,
    USOCK_Pending = 2,
    USOCK_Open = 3,
};

class UNetConnection : public UPlayer
{
public:
    TArray<UChildConnection*>& GetChildren();

    UNetDriver* GetDriver() const;

    UPackageMap* GetPackageMap() const;

    TArray<AActor*>& GetSentTemporaries();

    AActor* GetViewTarget() const;

    void SetViewTarget(AActor* InViewTarget);

    AActor* GetOwningActor() const;

    bool GetInternalAck() const;

    int32& GetQueuedBits();

    bool IsReplay() const;

    TArray<UChannel*>& GetChannels();

    bool IsReplayReady() const;

    void Close();

    AActor*& GetRepContextActor();

    ULevel*& GetRepContextLevel();

    bool GetPendingCloseDueToReplicationFailure();

    EConnectionState GetState() const;

    double GetLastReceiveTime() const;

    int32 GetTickCount() const;

    void SetTickCount(int32 InTickCount);

    void SetTimeSensitive(bool bInTimeSensitive);

    TMap<TWeakObjectPtr<AActor>, UActorChannel*, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, UActorChannel*>>& GetActorChannels();

    TMap<TWeakObjectPtr<AActor>, double, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, double>>& GetLastUpdateTimes();

    TSet<FNetworkGUID>& GetDestroyedStartupOrDormantActors();

    TSet<FName>& GetClientVisibleLevelNames();

    int32 IsNetReady(bool Saturate);

    UChannel* CreateChannel(EChannelType Type, bool bOpenedLocally, int32 ChannelIndex = -1);

    UChannel* CreateChannelByName(const FName& ChName, EChannelCreateFlags CreateFlags, int32 ChIndex = -1);

    void FlushDormancy(AActor* Actor);

    UChildConnection* GetUChildConnection();

    UReplicationConnectionDriver*& GetReplicationConnectionDriver();
};

class UChildConnection : public UNetConnection
{
public:
    UNetConnection* GetParent() const;
};
