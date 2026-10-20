#include "pch.h"
#include "Athena/AI/FortAthenaAIBotCosmeticLibraryData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

TAssetPtr<UDataTable>& UFortAthenaAIBotCosmeticLibraryData::GetPredefineSetsDataTable()
{
    static auto PredefineSetsDataTable = GetClass()->FindPropertyByName(FName(TEXT("PredefineSetsDataTable")));

    return *PredefineSetsDataTable->ContainerPtrToValuePtr<TAssetPtr<UDataTable>>(this);
}
