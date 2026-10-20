#pragma once

#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "UObject/AssetPtr.h"

#include "Internationalization/Text.h"

class UFortAbilitySet;
class UFortWeaponItemDefinition;
class UFortResourceItemDefinition;
enum class EFortResourceType : uint8;
struct FFortLootPackageData;
struct FFortLootTierData;

class UFortGameData : public UPrimaryDataAsset
{
public:
    static UFortGameData* Get();

    UFortAbilitySet* GetGenericPlayerAbilitySet() const;

    UFortAbilitySet* GetAthenaPlayerAbilitySet() const;

    UFortResourceItemDefinition* GetResourceItemDefinition(EFortResourceType ResourceType) const;

    FText GetBuildingResourceName(EFortResourceType ResourceType) const;

    UFortWeaponItemDefinition* GetEditToolItem() const;

    const TAssetPtr<UClass>& GetEmoteGameplayAbility() const;

    const TAssetPtr<UClass>* GetSprayGameplayAbility() const;

    const TArray<TAssetPtr<UClass>>* GetAIDirectors() const;

    const TArray<TAssetPtr<UDataTable>>& GetLootTierDataTables() const;

    const TArray<TAssetPtr<UDataTable>>& GetLootPackageDataTables() const;

    FFortLootTierData* FindLootTierDataRow(FName TierGroupName, const FString& ContextString) const;

    bool HasLootPackageData() const;

    void GetLootTierDataTables(TArray<UDataTable*>& OutLootTierDataTables) const;

    void GetLootPackageDataTables(TArray<UDataTable*>& OutLootPackageDataTables) const;

    template <typename PredicateType>
    void FindLootTierDataByPredicate(TArray<TPair<FName, FFortLootTierData*>>& OutLootTierData, float& OutTotalWeight, PredicateType&& Predicate) const
    {
        TArray<UDataTable*> LootTierDataTables;

        GetLootTierDataTables(LootTierDataTables);

        FindLootDataTypeByPredicate(OutLootTierData, OutTotalWeight, LootTierDataTables, Predicate);
    }

    template <typename PredicateType>
    void FindLootPackageDataByPredicate(TArray<TPair<FName, FFortLootPackageData*>>& OutLootPackageData, float& OutTotalWeight, PredicateType&& Predicate) const
    {
        TArray<UDataTable*> LootPackageDataTables;

        GetLootPackageDataTables(LootPackageDataTables);

        FindLootDataTypeByPredicate(OutLootPackageData, OutTotalWeight, LootPackageDataTables, Predicate);
    }

    template <typename PredicateType, typename DataType>
    static void FindLootDataTypeByPredicate(TArray<TPair<FName, DataType*>>& OutLootData, float& OutTotalWeight, const TArray<UDataTable*>& DataTables, PredicateType& Predicate)
    {
        for (auto DataTable : DataTables)
        {
            if (DataTable && DataTable->RowStruct && DataTable->RowStruct->IsChildOf(DataType::StaticStruct()))
            {
                for (auto& RowPair : DataTable->RowMap)
                {
                    if (auto Row = (DataType*)RowPair.Value)
                    {
                        if (Predicate(*Row))
                        {
                            OutLootData.Add(TPair<FName, DataType*>{ RowPair.Key, Row });
                            OutTotalWeight += Row->Weight;
                        }
                    }
                }
            }
        }
    }
};
