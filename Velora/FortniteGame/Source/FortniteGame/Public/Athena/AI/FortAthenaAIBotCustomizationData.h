#pragma once

#include "Engine/DataAsset.h"
#include "UObject/AssetPtr.h"

class AFortPlayerPawnAthena;
class UFortAthenaAIBotCosmeticLibraryData;
class UScriptStruct;
struct FFortAthenaLoadout;
struct FGameplayTag;

enum class EBotDataOverrideCosmeticMode : uint8
{
    SpecificLoadout,
    CosmeticLibrary
};

struct FFortAthenaAIBotRunTimeCustomizationData
{
    static UScriptStruct* StaticStruct();

    const FGameplayTag& GetPredefinedCosmeticSetTag() const;
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

    bool GetbOverrideCharacterCustomization() const;

    EBotDataOverrideCosmeticMode GetOverrideCosmeticMode() const;

    UFortAthenaAIBotCharacterCustomization* GetCharacterCustomization() const;

    TAssetPtr<UFortAthenaAIBotCosmeticLibraryData>& GetCosmeticCustomizationLibrary();
};
