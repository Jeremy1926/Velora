#include "pch.h"
#include "FortGameMode.h"
#include "FortGameInstance.h"
#include "FortGameData.h"
#include "Engine/DataTable.h"
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

        while (Scanner::IsCode(VTable[Index]) && !Scanner::At(VTable[Index]).Is("mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*") && !Scanner::At(VTable[Index]).Is("mov rax, [rcx]; mov r9, r8; mov r8, rdx; mov dl, *; jmp [rax+*]"))
            Index++;

        auto Forwarder = Scanner::At(Scanner::IsCode(VTable[Index]) ? VTable[Index] : nullptr);
        auto BuildingAction = Forwarder.Is("mov rax, [rcx]*") ? std::format("call [rax+{:#x}]", Forwarder.Find("jmp [rax+*]").Slot(TEXT("AFortGameMode::OnPlayerBuildingAction"))) : std::format("call {:#x}", (uintptr_t)Forwarder.Find("jmp 0x*").Target(TEXT("AFortGameMode::OnPlayerBuildingAction")));

        return Scanner::Slot(TEXT("AFortGameMode::OnPlayerCreatedBuilding"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Start().Has(BuildingAction); });
    }();

    return OnPlayerCreatedBuilding;
}

static int32_t GetLootTierDataTablesSlot()
{
    static auto GetLootTierDataTables = []
    {
        auto Playlist = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist"));
        auto LootTierData = Playlist ? Playlist->FindPropertyByName(FName(TEXT("LootTierData"))) : nullptr;
        auto LootPackages = Playlist ? Playlist->FindPropertyByName(FName(TEXT("LootPackages"))) : nullptr;
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Image = Scanner::Base();
        auto ImageEnd = Image + ((IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew))->OptionalHeader.SizeOfImage;

        auto Reads = [&](int32 Index, UProperty* Property)
        {
            return VTable[Index] >= Image && VTable[Index] < ImageEnd && (Scanner::At(VTable[Index]).Has(std::format("add rcx, {:#x}", Property->GetOffset_ForInternal())) || Scanner::At(VTable[Index]).Has(std::format("lea rcx, [*+{:#x}]", Property->GetOffset_ForInternal())));
        };

        for (int32 Index = 0; LootTierData && LootPackages && Scanner::IsCode(VTable[Index + 1]); Index++)
        {
            if (Reads(Index, LootTierData) && !Reads(Index, LootPackages) && Reads(Index + 1, LootPackages) && !Reads(Index + 1, LootTierData))
                return Scanner::Slot(TEXT("AFortGameModeAthena::GetLootTierDataTables"), Index * (int32)sizeof(void*));
        }

        return 0;
    }();

    return GetLootTierDataTables;
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

void AFortGameMode::GetLootTierDataTables(TArray<UDataTable*>& OutLootTierDataTables) const
{
    if (Scanner::IsSlot(GetLootTierDataTablesSlot()))
    {
        Scanner::Virtual<void (*)(const AFortGameMode*, TArray<UDataTable*>&)>(this, GetLootTierDataTablesSlot())(this, OutLootTierDataTables);
        return;
    }

    for (auto& LootTierDataTable : UFortGameData::Get()->GetLootTierDataTables())
    {
        if (auto DataTable = LootTierDataTable.LoadSynchronous())
            OutLootTierDataTables.Add(DataTable);
    }
}

void AFortGameMode::GetLootPackageDataTables(TArray<UDataTable*>& OutLootPackageDataTables) const
{
    if (Scanner::IsSlot(GetLootTierDataTablesSlot()))
    {
        Scanner::Virtual<void (*)(const AFortGameMode*, TArray<UDataTable*>&)>(this, GetLootTierDataTablesSlot(), sizeof(void*))(this, OutLootPackageDataTables);
        return;
    }

    for (auto& LootPackageDataTable : UFortGameData::Get()->GetLootPackageDataTables())
    {
        if (auto DataTable = LootPackageDataTable.LoadSynchronous())
            OutLootPackageDataTables.Add(DataTable);
    }
}
