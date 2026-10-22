#include "pch.h"
#include "FortGameStateComponent_Campsite.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static TArray<FName> ClaimedBotNames;

UClass* UFortGameStateComponent_Campsite::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CampsiteRuntime.FortGameStateComponent_Campsite"));

    return Class;
}

UFortGameStateComponent_Campsite* UFortGameStateComponent_Campsite::Get(const UObject* WorldContextObject)
{
    auto World = WorldContextObject && StaticClass() ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? (AActor*)World->GetGameState() : nullptr;

    return GameState ? (UFortGameStateComponent_Campsite*)GameState->FindComponentByClass(StaticClass()) : nullptr;
}

void UFortGameStateComponent_Campsite::execClaimUnusedBotName(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGameStateComponent_Campsite, execClaimUnusedBotName, Anchor::Hook()
        .If(StaticClass())
        .Exec(TEXT("/Script/CampsiteRuntime.FortGameStateComponent_Campsite:ClaimUnusedBotName"))
        .Find("setnz *; add [*+0x20], *; test rcx, rcx; jz *; call 0x*; add rsp, *; pop *; ret")
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortGameStateComponent_Campsite:ClaimUnusedBotName")));

    FString TemporaryBotName;
    auto& OutBotName = Stack.StepCompiledInRef<FString>(&TemporaryBotName);

    Stack.Code += !!Stack.Code;

    ClaimUnusedBotName(OutBotName);
}

void UFortGameStateComponent_Campsite::ClaimUnusedBotName(FString& OutBotName)
{
    static auto UseBotNames = IConsoleManager::Get().FindConsoleVariable(TEXT("Campsite.UseBotNames"));
    static auto BotNameDataTable = StaticClass()->FindPropertyByName(FName(TEXT("BotNameDataTable")));
    static auto BotNameDataAsset = StaticClass()->FindPropertyByName(FName(TEXT("BotNameDataAsset")));
    static auto DefaultNameDataTable = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIBotNameDataAsset")))->FindPropertyByName(FName(TEXT("DefaultNameDataTable")));
    static auto Name = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIBotNameDataTableRow")))->FindPropertyByName(FName(TEXT("Name")));

    if ((UseBotNames && !UseBotNames->GetInt()) || !BotNameDataTable || !BotNameDataAsset || !DefaultNameDataTable || !Name)
        return;

    auto& DataTable = *BotNameDataTable->ContainerPtrToValuePtr<UDataTable*>(this);

    if (!DataTable)
    {
        auto NameDataAsset = BotNameDataAsset->ContainerPtrToValuePtr<TSoftObjectPtr<UObject>>(this)->LoadSynchronous();

        DataTable = NameDataAsset ? DefaultNameDataTable->ContainerPtrToValuePtr<TSoftObjectPtr<UDataTable>>(NameDataAsset)->LoadSynchronous() : nullptr;
        ClaimedBotNames.Empty();
    }

    if (!DataTable)
        return;

    TArray<FName> UnclaimedRowNames;

    for (auto& RowPair : DataTable->RowMap)
    {
        if (!ClaimedBotNames.Contains(RowPair.Key))
            UnclaimedRowNames.Add(RowPair.Key);
    }

    if (!UnclaimedRowNames.Num())
        return;

    const FName RowName = UnclaimedRowNames[FMath::RandHelper(UnclaimedRowNames.Num())];
    auto RowData = DataTable->RowMap.Find(RowName);

    ClaimedBotNames.Add(RowName);

    if (RowData && *RowData)
        OutBotName = *Name->ContainerPtrToValuePtr<FString>(*RowData);
}

TArray<AAbandonedCampsitePlacedSpawner*>& UFortGameStateComponent_Campsite::GetPlacedCampsiteSpawnPoints()
{
    static auto PlacedCampsiteSpawnPoints = StaticClass()->FindPropertyByName(FName(TEXT("PlacedCampsiteSpawnPoints")));

    return *PlacedCampsiteSpawnPoints->ContainerPtrToValuePtr<TArray<AAbandonedCampsitePlacedSpawner*>>(this);
}
