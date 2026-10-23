#pragma once

#include "Engine/Channel.h"

class AActor;
struct FActorDestructionInfo;

struct FScopedActorRoleSwap
{
    FScopedActorRoleSwap(AActor* InActor);

    ~FScopedActorRoleSwap();

    FScopedActorRoleSwap(const FScopedActorRoleSwap&) = delete;

    FScopedActorRoleSwap& operator=(const FScopedActorRoleSwap&) = delete;

private:
    AActor* Actor;
};

enum class ESetChannelActorFlags : uint32
{
    None = 0,
};

class UActorChannel : public UChannel
{
public:
    using UChannel::Close;

    AActor* GetActor() const;

    double GetRelevantTime() const;

    void SetRelevantTime(double InRelevantTime);

    double GetLastUpdateTime() const;

    bool GetSpawnAcked() const;

    void Close();

    void StartBecomingDormant();

    int64 ReplicateActor();

    void SetChannelActor(AActor* InActor, ESetChannelActorFlags Flags = ESetChannelActorFlags::None);

    int64 SetChannelActorForDestroy(FActorDestructionInfo* DestructInfo);
};
