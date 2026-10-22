#pragma once

#include "UObject/NameTypes.h"

struct FPrimaryAssetType
{
    FName Name;
};

struct FPrimaryAssetId
{
    static FPrimaryAssetId ParseTypeAndName(const FString& TypeAndName);

    FPrimaryAssetType PrimaryAssetType;
    FName PrimaryAssetName;
};
