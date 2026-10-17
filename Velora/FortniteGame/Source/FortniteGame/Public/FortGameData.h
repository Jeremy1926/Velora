#pragma once

#include "Engine/DataAsset.h"

class UFortAbilitySet;

class UFortGameData : public UPrimaryDataAsset
{
public:
    static UFortGameData* Get();

    UFortAbilitySet* GetGenericPlayerAbilitySet() const;
};
