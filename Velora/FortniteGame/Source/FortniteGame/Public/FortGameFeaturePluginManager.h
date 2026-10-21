#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class UFortGameFeatureData;

class UFortGameFeaturePluginManager : public UObject
{
public:
    static UFortGameFeaturePluginManager* Get();

    void GetGameFeatureDataForActivePlugins(TArray<UFortGameFeatureData*>& OutGameFeatureDatas) const;
};
