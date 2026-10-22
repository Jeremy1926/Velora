#pragma once

#include "UObject/Object.h"

class UWorld;

class UWaterSubsystem : public UObject
{
public:
    static UWaterSubsystem* GetWaterSubsystem(const UWorld* InWorld);

    float GetOceanFloodHeight() const;
};
