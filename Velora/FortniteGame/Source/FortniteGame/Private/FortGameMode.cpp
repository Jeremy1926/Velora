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

UClass* AFortGameMode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameMode"));

    return Class;
}

void AFortGameMode::OnPlayerCreatedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    static auto OnPlayerCreatedBuilding = Anchor::VirtualFunction<void (*)(AFortGameMode*, AFortPlayerController*, ABuildingActor*)>(TEXT("AFortGameMode::OnPlayerCreatedBuilding"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*", "mov rax, [rcx]; mov r9, r8; mov r8, rdx; mov dl, *; jmp [rax+*]")
        .Is("mov rax, [rcx]*")
        .Find("jmp [rax+*]")
        .LogSlot(TEXT("AFortGameMode::OnPlayerBuildingAction"))
        .VirtualSlotCaller(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov dl, *; mov rcx, *; ")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*", "mov rax, [rcx]; mov r9, r8; mov r8, rdx; mov dl, *; jmp [rax+*]")
        .Find("jmp 0x*")
        .Target()
        .Log(TEXT("AFortGameMode::OnPlayerBuildingAction"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Get();

    OnPlayerCreatedBuilding(this, PlayerController, BuildingActor);
}

void AFortGameMode::OnPlayerRepairedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    static auto OnPlayerRepairedBuilding = Anchor::VirtualFunction<void (*)(AFortGameMode*, AFortPlayerController*, ABuildingActor*)>(TEXT("AFortGameMode::OnPlayerCreatedBuilding"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*", "mov rax, [rcx]; mov r9, r8; mov r8, rdx; mov dl, *; jmp [rax+*]")
        .Is("mov rax, [rcx]*")
        .Find("jmp [rax+*]")
        .LogSlot(TEXT("AFortGameMode::OnPlayerBuildingAction"))
        .VirtualSlotCaller(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov dl, *; mov rcx, *; ")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*", "mov rax, [rcx]; mov r9, r8; mov r8, rdx; mov dl, *; jmp [rax+*]")
        .Find("jmp 0x*")
        .Target()
        .Log(TEXT("AFortGameMode::OnPlayerBuildingAction"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Get(3 * sizeof(void*));

    OnPlayerRepairedBuilding(this, PlayerController, BuildingActor);
}

void AFortGameMode::OnPlayerEditedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    static auto OnPlayerEditedBuilding = Anchor::VirtualFunction<void (*)(AFortGameMode*, AFortPlayerController*, ABuildingActor*)>(TEXT("AFortGameMode::OnPlayerCreatedBuilding"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*", "mov rax, [rcx]; mov r9, r8; mov r8, rdx; mov dl, *; jmp [rax+*]")
        .Is("mov rax, [rcx]*")
        .Find("jmp [rax+*]")
        .LogSlot(TEXT("AFortGameMode::OnPlayerBuildingAction"))
        .VirtualSlotCaller(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov dl, *; mov rcx, *; ")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*", "mov rax, [rcx]; mov r9, r8; mov r8, rdx; mov dl, *; jmp [rax+*]")
        .Find("jmp 0x*")
        .Target()
        .Log(TEXT("AFortGameMode::OnPlayerBuildingAction"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Get(4 * sizeof(void*));

    OnPlayerEditedBuilding(this, PlayerController, BuildingActor);
}

void AFortGameMode::FinishWorldInitialization()
{
    static auto FinishWorldInitialization = Anchor::VirtualFunction<void (*)(AFortGameMode*)>(TEXT("AFortGameMode::FinishWorldInitialization"))
        .String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases."))
        .Start()
        .Log(TEXT("AFortGameModeAthena::FinishWorldInitialization"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Get(TEXT("/Script/FortniteGame.Default__FortGameMode"));

    FinishWorldInitialization(this);
}

void AFortGameMode::OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions)
{
    static auto OnGivenMatchAssignment = Anchor::VirtualFunction<void (*)(AFortGameMode*, const TMap<FString, FString>&)>(TEXT("AFortGameMode::OnGivenMatchAssignment"))
        .String(TEXT("Playtest-%s"))
        .Start()
        .Log(TEXT("AFortGameMode::OnGivenMatchAssignment"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameMode"))
        .Get();

    OnGivenMatchAssignment(this, MatchOptions);
}

bool AFortGameMode::GetWorldIsReady() const
{
    static auto bWorldIsReady = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bWorldIsReady")));

    return bWorldIsReady->GetPropertyValue_InContainer(this);
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

    return CurrentPlaylistName->GetPropertyValue_InContainer<FName>(this);
}

void AFortGameMode::GetLootTierDataTables(TArray<UDataTable*>& OutLootTierDataTables) const
{
    static auto Playlist = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist"));
    static auto GameModeAthena = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));
    static auto GetLootTierDataTables = Anchor::VirtualFunction<void (*)(const AFortGameMode*, TArray<UDataTable*>&)>(TEXT("AFortGameModeAthena::GetLootTierDataTables"))
        .FieldReaderPair(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), Playlist ? Playlist->FindPropertyByName(FName(TEXT("LootTierData"))) : nullptr, Playlist ? Playlist->FindPropertyByName(FName(TEXT("LootPackages"))) : nullptr)
        .Or()
        .FieldPattern("4C 8B B6 %o", GameModeAthena ? GameModeAthena->FindPropertyByName(FName(TEXT("GameFeatureLootTierData"))) : nullptr)
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Or()
        .FieldPattern("4C 8B BF %o", GameModeAthena ? GameModeAthena->FindPropertyByName(FName(TEXT("GameFeatureLootTierData"))) : nullptr)
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "jle 0x*; mov *, [*+%x]; movsxd *", GameModeAthena ? GameModeAthena->FindPropertyByName(FName(TEXT("GameFeatureLootTierData"))) : nullptr)
        .Optional()
        .Get();

    if (GetLootTierDataTables)
    {
        GetLootTierDataTables(this, OutLootTierDataTables);
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
    static auto Playlist = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist"));
    static auto GameModeAthena = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));
    static auto GetLootPackageDataTables = Anchor::VirtualFunction<void (*)(const AFortGameMode*, TArray<UDataTable*>&)>(TEXT("AFortGameModeAthena::GetLootTierDataTables"))
        .FieldReaderPair(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), Playlist ? Playlist->FindPropertyByName(FName(TEXT("LootTierData"))) : nullptr, Playlist ? Playlist->FindPropertyByName(FName(TEXT("LootPackages"))) : nullptr)
        .Or()
        .FieldPattern("4C 8B B6 %o", GameModeAthena ? GameModeAthena->FindPropertyByName(FName(TEXT("GameFeatureLootTierData"))) : nullptr)
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Or()
        .FieldPattern("4C 8B BF %o", GameModeAthena ? GameModeAthena->FindPropertyByName(FName(TEXT("GameFeatureLootTierData"))) : nullptr)
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Or()
        .FieldUser(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "jle 0x*; mov *, [*+%x]; movsxd *", GameModeAthena ? GameModeAthena->FindPropertyByName(FName(TEXT("GameFeatureLootTierData"))) : nullptr)
        .Optional()
        .Get(sizeof(void*));

    if (GetLootPackageDataTables)
    {
        GetLootPackageDataTables(this, OutLootPackageDataTables);
        return;
    }

    for (auto& LootPackageDataTable : UFortGameData::Get()->GetLootPackageDataTables())
    {
        if (auto DataTable = LootPackageDataTable.LoadSynchronous())
            OutLootPackageDataTables.Add(DataTable);
    }
}
