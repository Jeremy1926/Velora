#pragma once

#include "Engine/DataAsset.h"

class UFortGameDeathPenalty : public UDataAsset
{
public:
    static UClass* StaticClass();

    float GetSelfResurrectDurabilityPenalty() const;

    float GetTeammateResurrectDurabilityPenalty() const;
};
