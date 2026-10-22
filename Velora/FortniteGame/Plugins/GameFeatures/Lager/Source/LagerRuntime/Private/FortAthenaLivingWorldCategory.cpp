#include "pch.h"
#include "FortAthenaLivingWorldCategory.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FFortAthenaLivingWorldCategory::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldCategory"));

    return Struct;
}

TSoftObjectPtr<UDataTable>& FFortAthenaLivingWorldCategory::GetEventTable()
{
    static auto EventTable = StaticStruct()->FindPropertyByName(FName(TEXT("EventTable")));

    return *EventTable->ContainerPtrToValuePtr<TSoftObjectPtr<UDataTable>>(this);
}

int32& FFortAthenaLivingWorldCategory::GetMaxCount()
{
    static auto MaxCount = StaticStruct()->FindPropertyByName(FName(TEXT("MaxCount")));

    return *MaxCount->ContainerPtrToValuePtr<int32>(this);
}

float FFortAthenaLivingWorldCategory::GetPriority()
{
    static auto Priority = StaticStruct()->FindPropertyByName(FName(TEXT("Priority")));

    return Priority ? *Priority->ContainerPtrToValuePtr<float>(this) : 0.0f;
}

bool FFortAthenaLivingWorldCategory::GetbExcludeFromGlobalAIBudget()
{
    static auto bExcludeFromGlobalAIBudget = StaticStruct()->FindPropertyByName(FName(TEXT("bExcludeFromGlobalAIBudget")));

    return bExcludeFromGlobalAIBudget ? *bExcludeFromGlobalAIBudget->ContainerPtrToValuePtr<bool>(this) : false;
}

FGameplayTagContainer& FFortAthenaLivingWorldCategory::GetRequirePlaylistTags()
{
    static auto RequirePlaylistTags = StaticStruct()->FindPropertyByName(FName(TEXT("RequirePlaylistTags")));

    return *RequirePlaylistTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}
