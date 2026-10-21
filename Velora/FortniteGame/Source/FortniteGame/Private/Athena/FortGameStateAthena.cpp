#include "pch.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Athena/Vehicles/FortVehicleManager.h"
#include "Building/BuildingItemWeaponUpgradeActor.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldVolume.h"
#include "Online/FortPlaylistManager.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Misc/CoreMiscDefines.h"
#include "Templates/Casts.h"
#include "UObject/AssetPtr.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

PATCH(AFortGameStateAthena, BeginPlay, Anchor::Patch()
    .String(TEXT("GS BeginPlay"))
    .Optional()
    .Start()
    .Find("mov dl, 0x1; mov rcx, rax; mov r8, [rax]; call [r8+0x50]")
    .JumpOver("call [r8+0x50]"))

PATCH(AFortGameStateAthena, UpdateGamePhaseStep, Anchor::Patch()
    .String(TEXT("Gamephase Step: %s"))
    .Optional()
    .Start()
    .Find("call 0x*")
    .Is("call 0x*; test al, al; jz 0x*")
    .Find("test al, al")
    .Or()
    .String(TEXT("Gamephase Step: %s"))
    .Start()
    .Find("movzx *, byte ptr [*+0x40]; test *, *; jz 0x*")
    .Find("test *, *; jnz 0x*")
    .JumpOver("j*z 0x*"))

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

int32 AFortGameStateAthena::GetTeamsLeft() const
{
    static auto TeamsLeft = GetClass()->FindPropertyByName(FName(TEXT("TeamsLeft")));

    return *TeamsLeft->ContainerPtrToValuePtr<int32>(this);
}

EAthenaGamePhaseStep AFortGameStateAthena::GetGamePhaseStep() const
{
    static auto GamePhaseStep = GetClass()->FindPropertyByName(FName(TEXT("GamePhaseStep")));

    return *GamePhaseStep->ContainerPtrToValuePtr<EAthenaGamePhaseStep>(this);
}

int32 AFortGameStateAthena::GetSafeZonePhase() const
{
    static auto SafeZonePhase = GetClass()->FindPropertyByName(FName(TEXT("SafeZonePhase")));

    return *SafeZonePhase->ContainerPtrToValuePtr<uint8>(this);
}

UObject* AFortGameStateAthena::GetCurrentPlaylist() const
{
    static auto CurrentPlaylistData = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistData")));
    static auto CurrentPlaylistInfo = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistInfo")));
    static auto BasePlaylist = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("BasePlaylist"))) : nullptr;
    static auto OverridePlaylist = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("OverridePlaylist"))) : nullptr;

    if (!CurrentPlaylistInfo)
        return *CurrentPlaylistData->ContainerPtrToValuePtr<UObject*>(this);

    auto PlaylistInfo = CurrentPlaylistInfo->ContainerPtrToValuePtr<void>(this);

    if (OverridePlaylist && *OverridePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo))
        return *OverridePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo);

    return *BasePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo);
}

int32 AFortGameStateAthena::GetAvgPlayerMMR() const
{
    static auto GetAvgPlayerMMR = Anchor::Function<int32 (*)(const AFortGameStateAthena*)>(TEXT("AFortGameStateAthena::GetAvgPlayerMMR"))
        .Native(TEXT("/Script/FortniteGame.FortGameStateAthena:GetAvgPlayerMMR"))
        .Get();

    return GetAvgPlayerMMR(this);
}

AFortAthenaMapInfo* AFortGameStateAthena::GetMapInfo() const
{
    static auto MapInfo = GetClass()->FindPropertyByName(FName(TEXT("MapInfo")));

    return MapInfo ? *MapInfo->ContainerPtrToValuePtr<AFortAthenaMapInfo*>(this) : nullptr;
}

void AFortGameStateAthena::OnPlaylistLootTableAssetsDataLoadCompleted()
{
    DETOUR(AFortGameStateAthena, OnPlaylistLootTableAssetsDataLoadCompleted, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:SpawnVehicle")) || ABuildingItemWeaponUpgradeActor::StaticClass())
        .FieldPattern("66 C7 ? %o 01 00", AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded"))))
        .Start()
        .Inline());

    if (UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
    {
        if (auto LivingWorldManager = (UFortAthenaLivingWorldManager*)FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass()))
        {
            LivingWorldManager->GetCachedGameState() = this;

            FScriptDelegate OnCurrentPlaylistLoadedDelegate;
            OnCurrentPlaylistLoadedDelegate.BindUFunction(LivingWorldManager, FName(TEXT("OnCurrentPlaylistLoaded")));
            GetOnCurrentPlaylistLoaded().AddUnique(OnCurrentPlaylistLoadedDelegate);

            TArray<AActor*> LivingWorldVolumes;
            UGameplayStatics::GetAllActorsOfClass(this, AFortAthenaLivingWorldVolume::StaticClass(), LivingWorldVolumes);

            for (auto LivingWorldVolume : LivingWorldVolumes)
            {
                FScriptDelegate VolumeOnCurrentPlaylistLoadedDelegate;
                VolumeOnCurrentPlaylistLoadedDelegate.BindUFunction(LivingWorldVolume, FName(TEXT("OnCurrentPlaylistLoaded")));
                GetOnCurrentPlaylistLoaded().AddUnique(VolumeOnCurrentPlaylistLoadedDelegate);
            }
        }
    }

    Original();

    if (StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:SpawnVehicle")))
    {
        AFortVehicleManager* VehicleManager = nullptr;

        if (AFortVehicleManager::GetVehicleManager(this, VehicleManager))
        {
            VehicleManager->UpdateVehicleSpawns();
        }
        else
        {
            TArray<AActor*> VehicleSpawners;
            UGameplayStatics::GetAllActorsOfClass(this, AFortAthenaVehicleSpawner::StaticClass(), VehicleSpawners);

            for (auto VehicleSpawner : VehicleSpawners)
                ((AFortAthenaVehicleSpawner*)VehicleSpawner)->SpawnVehicle();
        }
    }

    if (ABuildingItemWeaponUpgradeActor::StaticClass())
    {
        TArray<AActor*> WeaponUpgradeActors;
        UGameplayStatics::GetAllActorsOfClass(this, ABuildingItemWeaponUpgradeActor::StaticClass(), WeaponUpgradeActors);

        for (auto WeaponUpgradeActor : WeaponUpgradeActors)
            ((ABuildingItemWeaponUpgradeActor*)WeaponUpgradeActor)->AthenaPostApplyPlaylistDataTables();
    }
}

FMulticastScriptDelegate& AFortGameStateAthena::GetOnCurrentPlaylistLoaded()
{
    static auto OnCurrentPlaylistLoaded = GetClass()->FindPropertyByName(FName(TEXT("OnCurrentPlaylistLoaded")));

    return *OnCurrentPlaylistLoaded->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
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
    static auto OnRep_CurrentPlaylistData = Anchor::Function<void (*)(AFortGameStateAthena*)>(TEXT("AFortGameStateAthena::OnRep_CurrentPlaylistData"))
        .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:OnRep_CurrentPlaylistData"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    OnRep_CurrentPlaylistData(this);
}

void AFortGameStateAthena::OnRep_CurrentPlaylistInfo()
{
    static auto OnRep_CurrentPlaylistInfo = Anchor::Function<void (*)(AFortGameStateAthena*)>(TEXT("AFortGameStateAthena::OnRep_CurrentPlaylistInfo"))
        .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:OnRep_CurrentPlaylistInfo"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    OnRep_CurrentPlaylistInfo(this);
}

void AFortGameStateAthena::StreamAdditionalPlaylistLevels()
{
    static auto CurrentPlaylistData = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistData")));
    static auto CurrentPlaylistInfo = GetClass()->FindPropertyByName(FName(TEXT("CurrentPlaylistInfo")));
    static auto BasePlaylist = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("BasePlaylist"))) : nullptr;
    static auto OverridePlaylist = CurrentPlaylistInfo ? ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PlaylistPropertyArray")))->FindPropertyByName(FName(TEXT("OverridePlaylist"))) : nullptr;
    static auto AdditionalLevels = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist")))->FindPropertyByName(FName(TEXT("AdditionalLevels")));
    static auto AdditionalLevelsServerOnly = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist")))->FindPropertyByName(FName(TEXT("AdditionalLevelsServerOnly")));

    auto PlaylistInfo = CurrentPlaylistInfo ? CurrentPlaylistInfo->ContainerPtrToValuePtr<void>(this) : nullptr;
    auto Playlist = PlaylistInfo ? *OverridePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo) : *CurrentPlaylistData->ContainerPtrToValuePtr<UObject*>(this);

    if (PlaylistInfo && !Playlist)
        Playlist = *BasePlaylist->ContainerPtrToValuePtr<UObject*>(PlaylistInfo);

    if (!HasAuthority() || !Playlist)
        return;

    for (auto& Level : *AdditionalLevels->ContainerPtrToValuePtr<TArray<TAssetPtr<UWorld>>>(Playlist))
    {
        if (!Level.IsNull())
            StartStreamingAdditionalPlaylistLevel(FName(*Level.GetLongPackageName()), false);
    }

    if (!AdditionalLevelsServerOnly)
        return;

    for (auto& Level : *AdditionalLevelsServerOnly->ContainerPtrToValuePtr<TArray<TAssetPtr<UWorld>>>(Playlist))
    {
        if (!Level.IsNull())
            StartStreamingAdditionalPlaylistLevel(FName(*Level.GetLongPackageName()), true);
    }
}

bool AFortGameStateAthena::AmmoDroppedFromContainersForWeaponPickup()
{
    static auto bAmmoDroppedFromContainersForWeaponPickup = IConsoleManager::Get().FindConsoleVariable(TEXT("Athena.bAmmoDroppedFromContainersForWeaponPickup"));

    return bAmmoDroppedFromContainersForWeaponPickup && bAmmoDroppedFromContainersForWeaponPickup->GetInt() == 1;
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

bool AFortGameStateAthena::AdjustedGravityMultiplier(AFortPawn* Pawn, float& NewGravityVal, float& NewVehicleGravityVal, float& NewJumpZ, float& NewJumpHorizontalAcceleration, float& NewJumpHorizontalVelocity)
{
    static auto AdjustedGravityMultiplier = Anchor::Function<bool (*)(AFortGameStateAthena*, AFortPawn*, float*, float*, float*, float*, float*)>(TEXT("AFortGameStateAthena::AdjustedGravityMultiplier"))
        .Native(TEXT("/Script/FortniteGame.FortPawn:SetGravityJumpMultipliers"))
        .CallSite()
        .Last("call 0x*; test al, al; jz *")
        .Call()
        .Get();

    return AdjustedGravityMultiplier(this, Pawn, &NewGravityVal, &NewVehicleGravityVal, &NewJumpZ, &NewJumpHorizontalAcceleration, &NewJumpHorizontalVelocity);
}
