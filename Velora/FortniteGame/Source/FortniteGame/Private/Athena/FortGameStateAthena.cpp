#include "pch.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Athena/Vehicles/FortVehicleManager.h"
#include "Building/BuildingContainer.h"
#include "Building/BuildingItemWeaponUpgradeActor.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldVolume.h"
#include "Online/FortPlaylistManager.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "Engine/CurveTable.h"
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
    .Or()
    .String(TEXT("GS BeginPlay"))
    .Start()
    .Find("mov dl, 0x1; mov r8, [rcx+0x50]; mov rcx, rax; call r8")
    .JumpOver("call *"))

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
    .Or()
    .String(TEXT("Gamephase Step: %s"))
    .Start()
    .Find("call 0x*")
    .Is("call 0x*; xor *, *; test al, al; jz 0x*")
    .Find("test al, al")
    .JumpOver("j*z 0x*"))

template<>
UEnum* StaticEnum<EAthenaGamePhaseStep>()
{
    static auto Enum = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EAthenaGamePhaseStep"));

    return Enum;
}

UClass* AFortGameStateAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameStateAthena"));

    return Class;
}

AFortGameplayMutator* AFortGameStateAthena::GetMutatorByClass_Implementation(const AActor* ContextActor, TSubclassOf<AFortGameplayMutator> MutatorClass)
{
    static auto GetMutatorByClass = Anchor::VirtualFunction<AFortGameplayMutator* (*)(AFortGameStateAthena*, const AActor*, TSubclassOf<AFortGameplayMutator>)>(TEXT("AFortGameStateAthena::GetMutatorByClass_Implementation"))
        .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:GetMutatorByClass"))
        .Find("setnz *")
        .Find("call [rax+0x*]")
        .Get();

    return GetMutatorByClass(this, ContextActor, MutatorClass);
}

TMulticastDelegate<void, APlayerState*>& AFortGameStateAthena::GetNotifyOnPlayerStateAdded()
{
    static auto NotifyOnPlayerStateAdded = Anchor::Data<TMulticastDelegate<void, APlayerState*>*>(TEXT("AFortGameStateAthena::NotifyOnPlayerStateAdded"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameStateAthena"), "mov [rsp+0x8], rbx; mov [rsp+0x18], rbp; mov [rsp+0x20], rsi; push rdi; sub rsp, 0x20; mov rsi, rcx; mov rbx, rdx; lea rcx, [0x*]; call 0x*")
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyOnPlayerStateAdded;
}

TMulticastDelegate<void, APlayerState*>& AFortGameStateAthena::GetNotifyOnPlayerStateRemoved()
{
    return *(&GetNotifyOnPlayerStateAdded() + 1);
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

AFortSafeZoneIndicator* AFortGameStateAthena::GetSafeZoneIndicator() const
{
    static auto SafeZoneIndicator = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneIndicator")));

    return SafeZoneIndicator ? *SafeZoneIndicator->ContainerPtrToValuePtr<AFortSafeZoneIndicator*>(this) : nullptr;
}

float AFortGameStateAthena::GetSafeZonePauseTime() const
{
    static auto SafeZonePauseTime = GetClass()->FindPropertyByName(FName(TEXT("SafeZonePauseTime")));

    return SafeZonePauseTime ? *SafeZonePauseTime->ContainerPtrToValuePtr<float>(this) : -1.0f;
}

bool AFortGameStateAthena::IsSafeZoneTimerPaused() const
{
    static auto IsSafeZoneTimerPaused = Anchor::VirtualFunction<bool (*)(const AFortGameStateAthena*)>(TEXT("AFortGameStateAthena::IsSafeZoneTimerPaused"))
        .String(TEXT("AFortGameStateAthena::StartPlaylistTimer(%f) Set PlaylistSecondsRemainingCount to: %d"))
        .Start()
        .Find("call [rax+0x*]; test al, al")
        .Get();

    return IsSafeZoneTimerPaused(this);
}

void AFortGameStateAthena::SetSafeZoneTimerPaused(bool bPaused)
{
    DETOUR(AFortGameStateAthena, SetSafeZoneTimerPaused, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortSafeZoneIndicator, OnSafeZoneTimerPauseChanged))
        .String(TEXT("PAUSESAFEZONE"))
        .Find("setz dl; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov dl, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Optional()
        .Inline());

    const bool bWasPaused = IsSafeZoneTimerPaused();

    Original(bPaused);

    if (bWasPaused == bPaused)
        return;

    if (auto SafeZoneIndicator = GetSafeZoneIndicator())
        SafeZoneIndicator->OnSafeZoneTimerPauseChanged(bPaused && GetSafeZonePauseTime() >= 0.0f);
}

void AFortGameStateAthena::StartPlaylistTimer(float Seconds, bool bOption)
{
    DETOUR(AFortGameStateAthena, StartPlaylistTimer, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortSafeZoneIndicator, OnSafeZoneTimerPauseChanged))
        .String(TEXT("AFortGameStateAthena::StartPlaylistTimer(%f) Set PlaylistSecondsRemainingCount to: %d"))
        .Start()
        .Optional()
        .Inline());

    static auto PlaylistSecondsRemainingCount = (int32)Anchor::Offset(TEXT("AFortGameStateAthena::PlaylistSecondsRemainingCount"))
        .String(TEXT("AFortGameStateAthena::StartPlaylistTimer(%f) Set PlaylistSecondsRemainingCount to: %d"))
        .Start()
        .Find("cvttss2si eax, xmm0")
        .Find("*")
        .Disp();

    Original(Seconds, bOption);

    if (PlaylistSecondsRemainingCount <= 0 || *(int32*)((uint8_t*)this + PlaylistSecondsRemainingCount) <= 0 || !IsSafeZoneTimerPaused())
        return;

    if (auto SafeZoneIndicator = GetSafeZoneIndicator())
        SafeZoneIndicator->OnSafeZoneTimerPauseChanged(GetSafeZonePauseTime() >= 0.0f);
}

const void* AFortGameStateAthena::GetSafeZoneInterface() const
{
    static auto SafeZoneInterface = Anchor::Offset(TEXT("AFortGameStateAthena::SafeZoneInterface"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("call 0x*")
        .Target()
        .Find("add rcx, 0x*")
        .Imm();

    return SafeZoneInterface > 0 ? (const uint8*)this + SafeZoneInterface : nullptr;
}

EFortSafeZoneState AFortGameStateAthena::GetSafeZoneState() const
{
    static auto GetSafeZoneState = Anchor::VirtualFunction<EFortSafeZoneState (*)(const void*)>(TEXT("AFortGameStateAthena::GetSafeZoneState"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Get();

    auto SafeZoneInterface = GetSafeZoneInterface();

    return SafeZoneInterface ? GetSafeZoneState(SafeZoneInterface) : EFortSafeZoneState::None;
}

FVector AFortGameStateAthena::GetSafeZoneCenter() const
{
    static auto GetSafeZoneCenter = Anchor::VirtualFunction<TGameValue<FVector>* (*)(const void*, TGameValue<FVector>*)>(TEXT("AFortGameStateAthena::GetSafeZoneCenter"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]", 2)
        .Get();

    TGameValue<FVector> SafeZoneCenter;

    if (auto SafeZoneInterface = GetSafeZoneInterface())
        GetSafeZoneCenter(SafeZoneInterface, &SafeZoneCenter);

    return SafeZoneCenter.Get();
}

float AFortGameStateAthena::GetSafeZoneRadius() const
{
    static auto GetSafeZoneRadius = Anchor::VirtualFunction<float (*)(const void*)>(TEXT("AFortGameStateAthena::GetSafeZoneRadius"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]", 3)
        .Get();

    auto SafeZoneInterface = GetSafeZoneInterface();

    return SafeZoneInterface ? GetSafeZoneRadius(SafeZoneInterface) : 0.0f;
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
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:GetAvgPlayerMMR"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto DefaultMMR = (int32)Anchor::Offset(TEXT("AFortGameStateAthena::GetAvgPlayerMMR default"))
            .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:GetAvgPlayerMMR"))
            .Find("add [*+0x20], *")
            .Find("mov eax, 0x*")
            .Imm();

        static auto bOverrideMMR = Anchor::Data<const uint8*>(TEXT("AFortGameStateAthena::GetAvgPlayerMMR override"))
            .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:GetAvgPlayerMMR"))
            .Find("add [*+0x20], *")
            .Find("cmp byte ptr [0x*], 0x0")
            .Target()
            .Get();

        static auto OverrideMMR = Anchor::Data<const int32*>(TEXT("AFortGameStateAthena::GetAvgPlayerMMR value"))
            .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:GetAvgPlayerMMR"))
            .Find("add [*+0x20], *")
            .Find("cmovnz eax, [0x*]")
            .Target()
            .Get();

        return bOverrideMMR && OverrideMMR && *bOverrideMMR ? *OverrideMMR : DefaultMMR;
    }

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
            LivingWorldManager->SetCachedGameState(this);

            if (auto CachedWorld = LivingWorldManager->GetCachedWorld())
                *CachedWorld = GetWorld();

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

    if (auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode()))
        GameMode->PerformAsyncLoadForGameFeatureLootData();

    Original();

    TArray<AActor*> BuildingContainers;
    UGameplayStatics::GetAllActorsOfClass(this, ABuildingContainer::StaticClass(), BuildingContainers);

    for (auto Actor : BuildingContainers)
    {
        auto BuildingContainer = (ABuildingContainer*)Actor;

        if (BuildingContainer->GetStartAlreadySearched_Athena())
            BuildingContainer->SpawnLoot(nullptr);
    }

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

void AFortGameStateAthena::ApplyPlaylistDataTables()
{
    DETOUR(AFortGameStateAthena, ApplyPlaylistDataTables, Anchor::Hook()
        .If(!::Anchor::Exists().String(TEXT("Default.Parachute.DeployTraceForGroundDistance")).Get())
        .String(TEXT("PLAYLIST: Preparing to apply Data Tables and Curves in AFortGameModeAthena::ApplyPlaylistDataTables(), PlaylistName is %s (Server Only)"))
        .Start()
        .Optional()
        .Inline());

    Original();

    static auto AthenaGameDataTable = GetClass()->FindPropertyByName(FName(TEXT("AthenaGameDataTable")));
    auto GameData = AthenaGameDataTable ? *AthenaGameDataTable->ContainerPtrToValuePtr<UCurveTable*>(this) : nullptr;

    if (!GameData)
        return;

    static const FString GliderRedeployContext(TEXT("HotfixWorkAround for Glider Redeploy"));
    static const FString AutoDeployContext(TEXT("HotfixWorkAround for Auto Deploy Height from Bus"));
    static const FString NeutralEditingContext(TEXT("HotfixWorkAround for Allow Neutral Editing"));
    static const FString RebootVanContext(TEXT("HotfixWorkAround for Spawn Reboot Van Hotfix"));

    auto Apply = [this, GameData](const TCHAR* RowName, const FString& ContextString, const TCHAR* PropertyName)
    {
        auto Property = GetClass()->FindPropertyByName(FName(PropertyName));

        if (auto Curve = GameData->FindCurve(FName(RowName), ContextString); Curve && Property)
            *Property->ContainerPtrToValuePtr<float>(this) = Curve->Eval(0.0f);
    };

    Apply(TEXT("Default.GliderRedeploy.CanRedeploy"), GliderRedeployContext, TEXT("DefaultGliderRedeployCanRedeploy"));
    Apply(TEXT("Default.RedeployGlider.LateralVelocityMult"), GliderRedeployContext, TEXT("DefaultRedeployGliderLateralVelocityMult"));
    Apply(TEXT("Default.RedeployGlider.HeightLimit"), GliderRedeployContext, TEXT("DefaultRedeployGliderHeightLimit"));
    Apply(TEXT("Default.Parachute.DeployTraceForGroundDistance"), AutoDeployContext, TEXT("DefaultParachuteDeployTraceForGroundDistance"));
    Apply(TEXT("AllowNeutralEditing"), NeutralEditingContext, TEXT("DefaultAllowNeutralWallEditing"));
    Apply(TEXT("Default.SCMachine.HotfixOn"), RebootVanContext, TEXT("DefaultRebootMachineHotfix"));
    Apply(TEXT("SignalInStormRegenSpeedCN"), GliderRedeployContext, TEXT("SignalInStormRegenSpeed"));
    Apply(TEXT("SignalInStormLostSpeedCN"), GliderRedeployContext, TEXT("SignalInStormLostSpeed"));

    if (auto Curve = GameData->FindCurve(FName(TEXT("DamageVulnerabilityInStormCN")), GliderRedeployContext))
    {
        const TCHAR* Levels[] = { TEXT("StormCNDamageVulnerabilityLevel0"), TEXT("StormCNDamageVulnerabilityLevel1"), TEXT("StormCNDamageVulnerabilityLevel2"), TEXT("StormCNDamageVulnerabilityLevel3") };

        for (int32 Level = 0; Level < 4; Level++)
        {
            if (auto Property = GetClass()->FindPropertyByName(FName(Levels[Level])))
                *Property->ContainerPtrToValuePtr<float>(this) = Curve->Eval((float)Level);
        }
    }
}

FGameplayTagContainer AFortGameStateAthena::GetAthenaPlaylistContextTags() const
{
    static auto GetAthenaPlaylistContextTags = Anchor::Function<FGameplayTagContainer* (*)(const AFortGameStateAthena*, FGameplayTagContainer*)>(TEXT("AFortGameStateAthena::GetAthenaPlaylistContextTags"))
        .String(TEXT(" - Adding preload object: '%s'"))
        .Start()
        .Find("call 0x*; mov rcx, rax; lea rdx, [*]; call 0x*; lea rdx, [*]; mov rcx, *; call 0x*")
        .Call(3)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:GetAthenaPlaylistContextTags"))
        .Call(1)
        .Optional()
        .Get();

    FGameplayTagContainer PlaylistContextTags;

    if (GetAthenaPlaylistContextTags)
        GetAthenaPlaylistContextTags(this, &PlaylistContextTags);

    return PlaylistContextTags;
}

FMulticastScriptDelegate& AFortGameStateAthena::GetOnCurrentPlaylistLoaded()
{
    static auto OnCurrentPlaylistLoaded = GetClass()->FindPropertyByName(FName(TEXT("OnCurrentPlaylistLoaded")));

    return *OnCurrentPlaylistLoaded->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

FMulticastScriptDelegate& AFortGameStateAthena::GetGamePhaseStepChanged()
{
    static auto GamePhaseStepChanged = GetClass()->FindPropertyByName(FName(TEXT("GamePhaseStepChanged")));

    return *GamePhaseStepChanged->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
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

bool AFortGameStateAthena::GetIsLargeTeamGame() const
{
    static auto bIsLargeTeamGame = GetClass()->FindPropertyByName(FName(TEXT("bIsLargeTeamGame")));

    return *bIsLargeTeamGame->ContainerPtrToValuePtr<bool>(this);
}

bool AFortGameStateAthena::IsCreativeOrPlaygroundModePlaylist() const
{
    static auto IsCreativeOrPlaygroundModePlaylist = Anchor::Function<bool (*)(const AFortGameStateAthena*)>(TEXT("AFortGameStateAthena::IsCreativeOrPlaygroundModePlaylist"))
        .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:IsCreativeOrPlaygroundModePlaylist"))
        .Call(1)
        .Get();

    return IsCreativeOrPlaygroundModePlaylist(this);
}

bool AFortGameStateAthena::IsBattleLabModePlaylist() const
{
    static auto IsBattleLabModePlaylist = Anchor::Function<bool (*)(UObject*)>(TEXT("UFortPlaylistAthenaHelper::IsBattleLabModePlaylist"))
        .Exec(TEXT("/Script/FortniteGame.FortGameStateAthena:IsBattleLabModePlaylist"))
        .Call(1)
        .Get();

    return IsBattleLabModePlaylist(GetCurrentPlaylist());
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

    if (Playlist && PlaylistNameProperty->GetPropertyValue_InContainer<FName>(Playlist) == PlaylistName && (*bPlaylistDataIsLoaded->ContainerPtrToValuePtr<bool>(this) || *bPlaylistDataIsActivelyLoading->ContainerPtrToValuePtr<bool>(this)))
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
        .Or()
        .Pattern("49 8B F0 49 8B F9 4C 8B C1 B3 01 44 8A CB 48 8D 48 B8 E8 ? ? ? ? 48 8B AC 24 B0 00 00 00 4C 8B B4 24 A8 00 00 00 48 8B 4C 24 60 48 85 C9 74 ? 48 8B 01 4C 8B C7 4C 8B 8C 24 A0 00 00 00 48 8B D6 48 89 6C 24 28 4C 89 74 24 20 FF 90 ? ? 00 00 3C 02")
        .Start()
        .Or()
        .Pattern("49 8B F8 4C 8B B4 24 ? ? ? ? 4C 8B C1 4C 8B BC 24 ? ? ? ? 48 8D 4C 24 ? 49 8B F1 B3 01 44 8A CB E8 ? ? ? ? 48 8B 8C 24 ? ? ? ? 48 85 C9 74 ? 48 8B 01 4C 8B CD 4C 89 7C 24 28 4C 8B C6 48 8B D7 4C 89 74 24 20 FF 90 ? ? 00 00 3C 02")
        .Start()
        .Get();

    return AdjustedGravityMultiplier(this, Pawn, &NewGravityVal, &NewVehicleGravityVal, &NewJumpZ, &NewJumpHorizontalAcceleration, &NewJumpHorizontalVelocity);
}

void AFortGameStateAthena::ShouldDropOrDestroyItem(bool* bCurrentCanDropValue, bool* bCurrentCanDestroyValue, AController* Controller, const UFortWorldItemDefinition* ItemDef)
{
    static auto ShouldDropOrDestroyItem = Anchor::Function<void (*)(AFortGameStateAthena*, bool*, bool*, AController*, const UFortWorldItemDefinition*)>(TEXT("AFortGameStateAthena::ShouldDropOrDestroyItem"))
        .At(DETOUR_ORIGINAL(AFortAthenaAIBotController, DropInventoryItems))
        .Find("mov [rsp+0x20], rbx")
        .Find("call 0x*")
        .Target()
        .Get();

    ShouldDropOrDestroyItem(this, bCurrentCanDropValue, bCurrentCanDestroyValue, Controller, ItemDef);
}

AFortBroadcastSpectatorInfo* AFortGameStateAthena::GetBroadcastSpectatorInfo() const
{
    static auto BroadcastSpectatorInfo = GetClass()->FindPropertyByName(FName(TEXT("BroadcastSpectatorInfo")));

    return *BroadcastSpectatorInfo->ContainerPtrToValuePtr<AFortBroadcastSpectatorInfo*>(this);
}
