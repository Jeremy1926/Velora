#include "pch.h"
#include "Athena/FortBotCosmeticItemDataTable.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FFortBotCosmeticItemSetDataTableRow::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortBotCosmeticItemSetDataTableRow"));

    return Struct;
}

FPrimaryAssetId& FFortBotCosmeticItemSetDataTableRow::GetCharacterAssetId()
{
    static auto CharacterAssetId = StaticStruct()->FindPropertyByName(FName(TEXT("CharacterAssetId")));

    return *CharacterAssetId->ContainerPtrToValuePtr<FPrimaryAssetId>(this);
}

FPrimaryAssetId& FFortBotCosmeticItemSetDataTableRow::GetGliderAssetId()
{
    static auto GliderAssetId = StaticStruct()->FindPropertyByName(FName(TEXT("GliderAssetId")));

    return *GliderAssetId->ContainerPtrToValuePtr<FPrimaryAssetId>(this);
}

FPrimaryAssetId& FFortBotCosmeticItemSetDataTableRow::GetSkyDiveContrailAssetId()
{
    static auto SkyDiveContrailAssetId = StaticStruct()->FindPropertyByName(FName(TEXT("SkyDiveContrailAssetId")));

    return *SkyDiveContrailAssetId->ContainerPtrToValuePtr<FPrimaryAssetId>(this);
}

FPrimaryAssetId& FFortBotCosmeticItemSetDataTableRow::GetBackpackAssetId()
{
    static auto BackpackAssetId = StaticStruct()->FindPropertyByName(FName(TEXT("BackpackAssetId")));

    return *BackpackAssetId->ContainerPtrToValuePtr<FPrimaryAssetId>(this);
}

FPrimaryAssetId& FFortBotCosmeticItemSetDataTableRow::GetPickaxeAssetId()
{
    static auto PickaxeAssetId = StaticStruct()->FindPropertyByName(FName(TEXT("PickaxeAssetId")));

    return *PickaxeAssetId->ContainerPtrToValuePtr<FPrimaryAssetId>(this);
}

float& FFortBotCosmeticItemSetDataTableRow::GetWeight()
{
    static auto Weight = StaticStruct()->FindPropertyByName(FName(TEXT("Weight")));

    return *Weight->ContainerPtrToValuePtr<float>(this);
}

FGameplayTag& FFortBotCosmeticItemSetDataTableRow::GetSetTag()
{
    static auto SetTag = StaticStruct()->FindPropertyByName(FName(TEXT("SetTag")));

    return *SetTag->ContainerPtrToValuePtr<FGameplayTag>(this);
}
