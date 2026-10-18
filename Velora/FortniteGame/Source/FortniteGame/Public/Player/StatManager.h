#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"

enum class EStatRecordingPeriod : uint8
{
    Minute,
    Life,
    Map,
    Campaign,
    Persistent,
    Max
};

enum class EStatMod : uint8
{
    Delta,
    Set,
    Maximum
};

class UStat : public UObject
{
public:
    void ModifyStat(int32 Amount, EStatMod ModType);
};

class UStatManager : public UObject
{
public:
    UStat* AddOrGetStat(FName StatName, EStatRecordingPeriod HighestPeriodToTrack);
};
