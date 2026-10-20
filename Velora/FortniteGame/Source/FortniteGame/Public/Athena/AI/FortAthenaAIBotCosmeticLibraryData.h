#pragma once

#include "Engine/DataAsset.h"
#include "UObject/AssetPtr.h"

class UDataTable;

class UFortAthenaAIBotCosmeticLibraryData : public UDataAsset
{
public:
    TAssetPtr<UDataTable>& GetPredefineSetsDataTable();
};
