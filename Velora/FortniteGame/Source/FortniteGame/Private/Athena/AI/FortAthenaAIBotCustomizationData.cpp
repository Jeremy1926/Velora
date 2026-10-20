#include "pch.h"
#include "Athena/AI/FortAthenaAIBotCustomizationData.h"
#include "Athena/AI/FortAthenaAIBotCosmeticLibraryData.h"
#include "Athena/FortBotCosmeticItemDataTable.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

FFortAthenaLoadout& UFortAthenaAIBotCharacterCustomization::GetCustomizationLoadout()
{
    static auto CustomizationLoadout = GetClass()->FindPropertyByName(FName(TEXT("CustomizationLoadout")));

    return *CustomizationLoadout->ContainerPtrToValuePtr<FFortAthenaLoadout>(this);
}

void UFortAthenaAIBotCustomizationData::GetOverrideCosmeticLoadout(AFortPlayerPawnAthena* Pawn, FFortAthenaLoadout& OutLoadout, const FGameplayTag& PredefinedCosmeticSetTag)
{
    DETOUR(UFortAthenaAIBotCustomizationData, GetOverrideCosmeticLoadout, Anchor::Hook()
        .String(TEXT("UFortServerBotManagerAthena::SpawnBot error occured during the actor spawning"))
        .Start()
        .Find("test *, 0x*; jz *; mov r9, *; lea r8, [*]; mov rdx, *; mov rcx, *; call 0x*")
        .Optional()
        .Find("call 0x*")
        .AtCall());

    FFortBotCosmeticItemSetDataTableRow* CosmeticSet = nullptr;

    if (GetOverrideCosmeticMode() == EBotDataOverrideCosmeticMode::CosmeticLibrary)
    {
        auto CosmeticLibrary = GetCosmeticCustomizationLibrary().LoadSynchronous();
        auto PredefineSetsDataTable = CosmeticLibrary ? CosmeticLibrary->GetPredefineSetsDataTable().LoadSynchronous() : nullptr;

        if (PredefineSetsDataTable)
        {
            float TotalWeight = 0.0f;

            for (auto& RowPair : PredefineSetsDataTable->RowMap)
            {
                auto Row = (FFortBotCosmeticItemSetDataTableRow*)RowPair.Value;

                if (Row->GetWeight() > 0.0f && Row->GetSetTag() == PredefinedCosmeticSetTag)
                    TotalWeight += Row->GetWeight();
            }

            float Pick = FMath::FRand() * TotalWeight;

            for (auto& RowPair : PredefineSetsDataTable->RowMap)
            {
                auto Row = (FFortBotCosmeticItemSetDataTableRow*)RowPair.Value;

                if (Row->GetWeight() <= 0.0f || !(Row->GetSetTag() == PredefinedCosmeticSetTag))
                    continue;

                CosmeticSet = Row;

                if (Pick < Row->GetWeight())
                    break;

                Pick -= Row->GetWeight();
            }
        }
    }

    if (!CosmeticSet)
    {
        if (auto CharacterCustomization = GetCharacterCustomization())
            FFortAthenaLoadout::StaticStruct()->CopyScriptStruct(&OutLoadout, &CharacterCustomization->GetCustomizationLoadout());

        return;
    }

    OutLoadout.GetCharacter() = (UAthenaCharacterItemDefinition*)UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(CosmeticSet->GetCharacterAssetId()).LoadSynchronous();
    OutLoadout.GetGlider() = (UAthenaGliderItemDefinition*)UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(CosmeticSet->GetGliderAssetId()).LoadSynchronous();
    OutLoadout.GetSkyDiveContrail() = (UAthenaSkyDiveContrailItemDefinition*)UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(CosmeticSet->GetSkyDiveContrailAssetId()).LoadSynchronous();
    OutLoadout.GetBackpack() = (UAthenaBackpackItemDefinition*)UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(CosmeticSet->GetBackpackAssetId()).LoadSynchronous();
    OutLoadout.GetPickaxe() = (UAthenaPickaxeItemDefinition*)UKismetSystemLibrary::GetSoftObjectReferenceFromPrimaryAssetId(CosmeticSet->GetPickaxeAssetId()).LoadSynchronous();
}

EBotDataOverrideCosmeticMode UFortAthenaAIBotCustomizationData::GetOverrideCosmeticMode() const
{
    static auto OverrideCosmeticMode = GetClass()->FindPropertyByName(FName(TEXT("OverrideCosmeticMode")));

    return *OverrideCosmeticMode->ContainerPtrToValuePtr<EBotDataOverrideCosmeticMode>(this);
}

UFortAthenaAIBotCharacterCustomization* UFortAthenaAIBotCustomizationData::GetCharacterCustomization() const
{
    static auto CharacterCustomization = GetClass()->FindPropertyByName(FName(TEXT("CharacterCustomization")));

    return *CharacterCustomization->ContainerPtrToValuePtr<UFortAthenaAIBotCharacterCustomization*>(this);
}

TAssetPtr<UFortAthenaAIBotCosmeticLibraryData>& UFortAthenaAIBotCustomizationData::GetCosmeticCustomizationLibrary()
{
    static auto CosmeticCustomizationLibrary = GetClass()->FindPropertyByName(FName(TEXT("CosmeticCustomizationLibrary")));

    return *CosmeticCustomizationLibrary->ContainerPtrToValuePtr<TAssetPtr<UFortAthenaAIBotCosmeticLibraryData>>(this);
}
