#include "pch.h"
#include "FortGameMode.h"
#include "FortGameInstance.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

static int32_t OnPlayerCreatedBuildingSlot()
{
    static auto OnPlayerCreatedBuilding = []
    {
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (Scanner::IsCode(VTable[Index]) && !Scanner::At(VTable[Index]).Is("mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*"))
            Index++;

        auto BuildingAction = std::format("call {:#x}", (uintptr_t)Scanner::At(Scanner::IsCode(VTable[Index]) ? VTable[Index] : nullptr).Find("jmp 0x*").Target(TEXT("AFortGameMode::OnPlayerBuildingAction")));

        return Scanner::Slot(TEXT("AFortGameMode::OnPlayerCreatedBuilding"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Start().Has(BuildingAction); });
    }();

    return OnPlayerCreatedBuilding;
}

UClass* AFortGameMode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameMode"));

    return Class;
}

void AFortGameMode::OnPlayerCreatedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    Scanner::Virtual<void (*)(AFortGameMode*, AFortPlayerController*, ABuildingActor*)>(this, OnPlayerCreatedBuildingSlot())(this, PlayerController, BuildingActor);
}

void AFortGameMode::OnPlayerRepairedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    Scanner::Virtual<void (*)(AFortGameMode*, AFortPlayerController*, ABuildingActor*)>(this, OnPlayerCreatedBuildingSlot(), 3 * sizeof(void*))(this, PlayerController, BuildingActor);
}

void AFortGameMode::OnPlayerEditedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    Scanner::Virtual<void (*)(AFortGameMode*, AFortPlayerController*, ABuildingActor*)>(this, OnPlayerCreatedBuildingSlot(), 4 * sizeof(void*))(this, PlayerController, BuildingActor);
}

void AFortGameMode::FinishWorldInitialization()
{
    static auto FinishWorldInitialization = []
    {
        auto AthenaFinishWorldInitialization = Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start().Get(TEXT("AFortGameModeAthena::FinishWorldInitialization"));
        auto AthenaVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto FinishWorldInitialization = Scanner::Slot(TEXT("AFortGameMode::FinishWorldInitialization"), AthenaVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == AthenaFinishWorldInitialization; });

        return Scanner::Virtual<void (*)(AFortGameMode*)>(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameMode")), FinishWorldInitialization);
    }();

    FinishWorldInitialization(this);
}

void AFortGameMode::OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions)
{
    static auto OnGivenMatchAssignment = []
    {
        auto FortGameModeOnGivenMatchAssignment = Scanner::String(TEXT("Playtest-%s")).Start().Get(TEXT("AFortGameMode::OnGivenMatchAssignment"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameMode"));

        return Scanner::Slot(TEXT("AFortGameMode::OnGivenMatchAssignment"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == FortGameModeOnGivenMatchAssignment; });
    }();

    Scanner::Virtual<void (*)(AFortGameMode*, const TMap<FString, FString>&)>(this, OnGivenMatchAssignment)(this, MatchOptions);
}

AFortGameSession* AFortGameMode::GetFortGameSession()
{
    static auto FortGameSession = GetClass()->FindPropertyByName(FName(TEXT("FortGameSession")));

    return *FortGameSession->ContainerPtrToValuePtr<AFortGameSession*>(this);
}

TAssetPtr<UFortGameDeathPenalty>& AFortGameMode::GetDeathPenaltyData()
{
    static auto DeathPenaltyData = GetClass()->FindPropertyByName(FName(TEXT("DeathPenaltyData")));

    return *DeathPenaltyData->ContainerPtrToValuePtr<TAssetPtr<UFortGameDeathPenalty>>(this);
}

UFortPlaylistManager* AFortGameMode::GetPlaylistManager() const
{
    static auto PlaylistManager = GetClass()->FindPropertyByName(FName(TEXT("PlaylistManager")));

    if (!PlaylistManager)
        return ((UFortGameInstance*)GetWorld()->GetGameInstance())->GetPlaylistManager();

    return *PlaylistManager->ContainerPtrToValuePtr<UFortPlaylistManager*>(this);
}

FName AFortGameMode::GetCurrentPlaylistName() const
{
    static auto CurrentPlaylistName = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistName")));

    return *CurrentPlaylistName->ContainerPtrToValuePtr<FName>(this);
}
