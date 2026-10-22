#pragma once

#include "Engine/AssetManager.h"
#include "Containers/UnrealString.h"

class UFortGameData;
class UGameDataBR;
class UGameDataCosmetics;

class UFortAssetManager : public UAssetManager
{
public:
    static UFortAssetManager& Get();

    UFortGameData* GetGameData();

    UGameDataBR* GetGameDataBR();

    UGameDataCosmetics* GetGameDataCosmetics();

    UObject* GetAssetForPersistentName(const FString& PersistentName);
};
