#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

struct FFortAthenaLivingWorldCategory : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    TSoftObjectPtr<UDataTable>& GetEventTable();

    int32& GetMaxCount();

    FGameplayTagContainer& GetRequirePlaylistTags();
};
