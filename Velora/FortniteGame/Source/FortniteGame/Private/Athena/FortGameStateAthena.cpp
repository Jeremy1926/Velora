#include "pch.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Online/FortPlaylistManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/CoreMiscDefines.h"
#include "Templates/Casts.h"
#include "UObject/AssetPtr.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto OnPlaylistLootTableAssetsDataLoadCompleted = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:SpawnVehicle")) ? []
{
    auto bPlaylistDataIsLoaded = (uint32)AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded")))->GetOffset_ForInternal();
    auto PlaylistDataIsLoaded = std::format("66 C7 ? {:02X} {:02X} {:02X} {:02X} 01 00", bPlaylistDataIsLoaded & 0xFF, (bPlaylistDataIsLoaded >> 8) & 0xFF, (bPlaylistDataIsLoaded >> 16) & 0xFF, bPlaylistDataIsLoaded >> 24);

    return Detour(TEXT("AFortGameStateAthena::OnPlaylistLootTableAssetsDataLoadCompleted"), Scanner::Pattern(PlaylistDataIsLoaded.c_str()).Start().Get(), &AFortGameStateAthena::OnPlaylistLootTableAssetsDataLoadCompleted);
}() : nullptr;

UClass* AFortGameStateAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameStateAthena"));

    return Class;
}

EAthenaGamePhase AFortGameStateAthena::GetGamePhase() const
{
    static auto GamePhase = GetClass()->FindPropertyByName(FName(TEXT("GamePhase")));

    return *GamePhase->ContainerPtrToValuePtr<EAthenaGamePhase>(this);
}

AFortAthenaMapInfo* AFortGameStateAthena::GetMapInfo() const
{
    static auto MapInfo = GetClass()->FindPropertyByName(FName(TEXT("MapInfo")));

    return MapInfo ? *MapInfo->ContainerPtrToValuePtr<AFortAthenaMapInfo*>(this) : nullptr;
}

void AFortGameStateAthena::OnPlaylistLootTableAssetsDataLoadCompleted()
{
    (this->*::OnPlaylistLootTableAssetsDataLoadCompleted)();

    TArray<AActor*> VehicleSpawners;
    UGameplayStatics::GetAllActorsOfClass(this, AFortAthenaVehicleSpawner::StaticClass(), VehicleSpawners);

    for (auto VehicleSpawner : VehicleSpawners)
        ((AFortAthenaVehicleSpawner*)VehicleSpawner)->SpawnVehicle();
}

FGameMemberInfoArray& AFortGameStateAthena::GetGameMemberInfoArray()
{
    static auto GameMemberInfoArray = GetClass()->FindPropertyByName(FName(TEXT("GameMemberInfoArray")));

    return *GameMemberInfoArray->ContainerPtrToValuePtr<FGameMemberInfoArray>(this);
}

bool AFortGameStateAthena::GetPlaylistDataIsLoaded() const
{
    static auto bPlaylistDataIsLoaded = GetClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded")));

    return *bPlaylistDataIsLoaded->ContainerPtrToValuePtr<bool>(this);
}

AFortVolumeManager* AFortGameStateAthena::GetVolumeManager() const
{
    static auto VolumeManager = GetClass()->FindPropertyByName(FName(TEXT("VolumeManager")));

    return *VolumeManager->ContainerPtrToValuePtr<AFortVolumeManager*>(this);
}

void AFortGameStateAthena::SetCurrentPlaylistName(FName PlaylistName)
{
    static auto CurrentPlaylistData = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistData")));
    static auto CurrentPlaylistInfo = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistInfo")));
    static auto BasePlaylist = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("BasePlaylist"))) : nullptr;
    static auto PlaylistReplicationKey = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("PlaylistReplicationKey"))) : nullptr;
    static auto bPlaylistDataIsLoaded = GetClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded")));
    static auto bPlaylistDataIsActivelyLoading = GetClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsActivelyLoading")));
    static auto AirCraftBehavior = GetClass()->FindPropertyByName(FName(TEXT("AirCraftBehavior")));
    static auto PlaylistNameProperty = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist")))->FindPropertyByName(FName(TEXT("PlaylistName")));
    static auto PlaylistAirCraftBehavior = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))->FindPropertyByName(FName(TEXT("AirCraftBehavior")));
    static auto PlaylistAISettings = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))->FindPropertyByName(FName(TEXT("AISettings")));

    auto PlaylistInfo = CurrentPlaylistInfo ? CurrentPlaylistInfo->ContainerPtrToValuePtr<void>(this) : nullptr;
    auto& Playlist = PlaylistInfo ? *BasePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo) : *CurrentPlaylistData->ContainerPtrToValuePtr<UObject*>(this);

    if (Playlist && *PlaylistNameProperty->ContainerPtrToValuePtr<FName>(Playlist) == PlaylistName && (*bPlaylistDataIsLoaded->ContainerPtrToValuePtr<bool>(this) || *bPlaylistDataIsActivelyLoading->ContainerPtrToValuePtr<bool>(this)))
        return;

    auto World = GetWorld();
    auto GameMode = World ? Cast<AFortGameModeAthena>(World->GetAuthGameMode()) : nullptr;

    if (!GameMode || !GameMode->GetPlaylistManager())
        return;

    auto NewPlaylist = GameMode->GetPlaylistManager()->GetAthenaPlaylist(PlaylistName);

    if (!NewPlaylist)
        return;

    if (PlaylistInfo)
    {
        auto& ReplicationKey = *PlaylistReplicationKey->ContainerPtrToValuePtr<int32>(PlaylistInfo);

        ReplicationKey++;
        Playlist = NewPlaylist;

        if (ReplicationKey == INDEX_NONE)
            ReplicationKey++;
    }
    else
    {
        Playlist = NewPlaylist;
    }

    *AirCraftBehavior->ContainerPtrToValuePtr<uint8>(this) = *PlaylistAirCraftBehavior->ContainerPtrToValuePtr<uint8>(NewPlaylist);

    if (auto AISettings = PlaylistAISettings ? *PlaylistAISettings->ContainerPtrToValuePtr<UAthenaAISettings*>(NewPlaylist) : nullptr)
        GameMode->OverrideAISettings(AISettings);

    if (PlaylistInfo)
        OnRep_CurrentPlaylistInfo();
    else
        OnRep_CurrentPlaylistData();

    StreamAdditionalPlaylistLevels();
}

void AFortGameStateAthena::OnRep_CurrentPlaylistData()
{
    static auto OnRep_CurrentPlaylistData = Scanner::Function<void (*)(AFortGameStateAthena*)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:OnRep_CurrentPlaylistData")).Find("jmp 0x*").Target(TEXT("AFortGameStateAthena::OnRep_CurrentPlaylistData")));

    OnRep_CurrentPlaylistData(this);
}

void AFortGameStateAthena::OnRep_CurrentPlaylistInfo()
{
    static auto OnRep_CurrentPlaylistInfo = Scanner::Function<void (*)(AFortGameStateAthena*)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:OnRep_CurrentPlaylistInfo")).Find("jmp 0x*").Target(TEXT("AFortGameStateAthena::OnRep_CurrentPlaylistInfo")));

    OnRep_CurrentPlaylistInfo(this);
}

void AFortGameStateAthena::StreamAdditionalPlaylistLevels()
{
    static auto CurrentPlaylistData = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistData")));
    static auto CurrentPlaylistInfo = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistInfo")));
    static auto BasePlaylist = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("BasePlaylist"))) : nullptr;
    static auto OverridePlaylist = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("OverridePlaylist"))) : nullptr;
    static auto AdditionalLevels = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist")))->FindPropertyByName(FName(TEXT("AdditionalLevels")));

    auto PlaylistInfo = CurrentPlaylistInfo ? CurrentPlaylistInfo->ContainerPtrToValuePtr<void>(this) : nullptr;
    auto Playlist = PlaylistInfo ? *OverridePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo) : *CurrentPlaylistData->ContainerPtrToValuePtr<UObject*>(this);

    if (PlaylistInfo && !Playlist)
        Playlist = *BasePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo);

    if (!HasAuthority() || !Playlist)
        return;

    for (auto& Level : *AdditionalLevels->ContainerPtrToValuePtr<TArray<TAssetPtr<UWorld>>>(Playlist))
    {
        if (!Level.IsNull())
            StartStreamingAdditionalPlaylistLevel(FName(*Level.GetLongPackageName()));
    }
}

UScriptStruct* FGameMemberInfoArray::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.GameMemberInfoArray"));

    return Struct;
}

TArray<FGameMemberInfo>& FGameMemberInfoArray::GetMembers()
{
    static auto Members = StaticStruct()->FindPropertyByName(FName(TEXT("Members")));

    return *Members->ContainerPtrToValuePtr<TArray<FGameMemberInfo>>(this);
}
