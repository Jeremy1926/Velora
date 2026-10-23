#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"

struct FFortBotCosmeticItemSetDataTableRow : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    FPrimaryAssetId GetCharacterAssetId();

    FPrimaryAssetId GetGliderAssetId();

    FPrimaryAssetId GetSkyDiveContrailAssetId();

    FPrimaryAssetId GetBackpackAssetId();

    FPrimaryAssetId GetPickaxeAssetId();

    float& GetWeight();

    FGameplayTag GetSetTag();
};
