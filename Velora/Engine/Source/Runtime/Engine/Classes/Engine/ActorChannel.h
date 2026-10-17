#pragma once

#include "Engine/Channel.h"

class AActor;
struct FActorDestructionInfo;

class UActorChannel : public UChannel
{
public:
    AActor* GetActor() const;

    double GetRelevantTime() const;

    void SetRelevantTime(double InRelevantTime);

    double GetLastUpdateTime() const;

    void Close();

    void StartBecomingDormant();

    bool ReplicateActor();

    void SetChannelActor(AActor* InActor);

    void SetChannelActorForDestroy(FActorDestructionInfo* DestructInfo);
};
