#pragma once

#include "Engine/AssetManager.h"

class UFortGameData;

class UFortAssetManager : public UAssetManager
{
public:
    static UFortAssetManager& Get();

    UFortGameData* GetGameData();
};
