#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class UGameFeatureData;

class UGameFeaturesSubsystem : public UObject
{
public:
    static UGameFeaturesSubsystem* Get();

    void GetGameFeatureDataForActivePlugins(TArray<const UGameFeatureData*>& OutActivePluginFeatureDatas);
};
