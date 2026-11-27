#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"
#include "Containers/Array.h"
#include "Engine/DataTable.h"

struct FGameplayTagQuery;

struct FAILootInfoRowEntry
{
    static UScriptStruct* StaticStruct();

    const FGameplayTagQuery& GetOptionalTagQuery() const;

    const FGameplayTagQuery& GetOptionalPlaylistTagQuery() const;

    bool GetbShouldDropInventoryOnDeath() const;

    bool GetbShouldDropLootOnDeath() const;

    bool GetbShouldBotPickRandomLootTier() const;

    const TArray<FName>& GetLootTiers() const;
};

struct FFortAthenaAILootInfoDataTableRow
{
    static UScriptStruct* StaticStruct();

    int32 GetNumLootDroppingBehaviors() const;

    const FAILootInfoRowEntry& GetLootDroppingBehavior(int32 Index) const;
};

class UFortAthenaAIRuntimeParameters_Inventory : public UObject
{
public:
    static UClass* StaticClass();

    bool GetbShouldDropCurrencyOnDeath() const;

    FName GetCurrencyPayoutRowName() const;

    const FDataTableRowHandle& GetLootInfo() const;
};
