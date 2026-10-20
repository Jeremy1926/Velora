#pragma once

#include "Engine/DataAsset.h"
#include "UObject/AssetPtr.h"

class AFortPlayerPawnAthena;
class UFortAthenaAIBotCosmeticLibraryData;
struct FFortAthenaLoadout;
struct FGameplayTag;

enum class EBotDataOverrideCosmeticMode : uint8
{
    SpecificLoadout,
    CosmeticLibrary
};

class UFortAthenaAIBotCharacterCustomization : public UObject
{
public:
    FFortAthenaLoadout& GetCustomizationLoadout();
};

class UFortAthenaAIBotCustomizationData : public UDataAsset
{
public:
    void GetOverrideCosmeticLoadout(AFortPlayerPawnAthena* Pawn, FFortAthenaLoadout& OutLoadout, const FGameplayTag& PredefinedCosmeticSetTag);

    EBotDataOverrideCosmeticMode GetOverrideCosmeticMode() const;

    UFortAthenaAIBotCharacterCustomization* GetCharacterCustomization() const;

    TAssetPtr<UFortAthenaAIBotCosmeticLibraryData>& GetCosmeticCustomizationLibrary();
};
