#pragma once

#include "UObject/NameTypes.h"

struct FPrimaryAssetType
{
    FName Name;
};

struct FPrimaryAssetId
{
    FPrimaryAssetType PrimaryAssetType;
    FName PrimaryAssetName;
};
