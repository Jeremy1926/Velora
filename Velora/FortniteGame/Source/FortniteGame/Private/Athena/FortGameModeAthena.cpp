#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "FortGameStateZone.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/AI/AthenaAISystem.h"
#include "Athena/AI/AthenaAIPerceptionManager.h"
#include "Athena/FortServerBotManagerAthena.h"
#include "Online/FortGameSession.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Building/BuildingContainer.h"
#include "Building/BGAConsumableSpawner.h"
#include "Building/BuildingItemCollectorActor.h"
#include "Building/BuildingSMActor.h"
#include "Building/FortVolumeManager.h"
#include "Weapons/FortMountedWeaponInterface.h"
#include "Weapons/FortWeapon.h"
#include "UObject/UObjectArray.h"
#include "Items/FortPickup.h"
#include "Player/FortPlayerState.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItemDefinition.h"
#include "FortniteGame.h"
#include "FortGameData.h"
#include "FortGameFeatureData.h"
#include "FortGameFeaturePluginManager.h"
#include "GameFeaturesSubsystem.h"
#include "Engine/DataTable.h"
#include "Abilities/FortAbilitySet.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/FortGameplayAbility.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "FortKismetLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Math/UnrealMathUtility.h"
#include "HAL/IConsoleManager.h"
#include "Templates/Casts.h"
#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortGameModeAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));

    return Class;
}

void AFortGameModeAthena::FinishWorldInitialization()
{
    DETOUR(AFortGameModeAthena, FinishWorldInitialization, Anchor::Hook()
        .If(!::Anchor::Exists()
            .String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases."))
            .Start()
            .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
            .Entry(TEXT("/Script/FortniteGame.Default__FortGamePvPBase"))
            .CallIn(::Anchor::Address()
                .String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases."))
                .Start()
                .Get())
            .Get())
        .String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases."))
        .Start()
        .Inline());

    AFortGameModeZone::FinishWorldInitialization();

    Original();
}

void AFortGameModeAthena::InitGameState()
{
    DETOUR(AFortGameModeAthena, InitGameState, Anchor::Hook()
        .If(AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("ServerBotManagerClass"))) != nullptr)
        .String(TEXT("&AFortGameModeAthena::OnGamePhaseStepChanged"))
        .Start()
        .Inline());

    Original();

    UClass* ServerBotManagerClass = GetServerBotManagerClass();
    auto ServerBotManager = NewObject<UFortServerBotManagerAthena>(this, ServerBotManagerClass ? ServerBotManagerClass : UFortServerBotManagerAthena::StaticClass());

    ServerBotManager->GetCachedGameMode() = this;
    ServerBotManager->GetCachedGameState() = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (auto AISystem = Cast<UAthenaAISystem>(GetWorld()->GetAISystem()))
        ServerBotManager->SetCachedAIPopulationTracker(AISystem->GetAIPopulationTracker());

    GetServerBotManager() = ServerBotManager;

    ServerBotManager->SetBotBrainActivationTimer();

    if (auto PhoebeEnable = IConsoleManager::Get().FindConsoleVariable(TEXT("Phoebe.Enable")))
        PhoebeEnable->Set(TEXT("0"), ECVF_SetByCode);
}

bool AFortGameModeAthena::ReadyToStartMatch_Implementation()
{
    DETOUR(AFortGameModeAthena, ReadyToStartMatch_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/Engine.GameMode:ReadyToStartMatch"))
        .Find("call [rax+*]")
        .HasVirtual(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "call 0x*; xor al, al")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__FortGameModeAthena")));

    if (GetWorldIsReady() && GetMatchState() == MatchState::WaitingToStart)
    {
        auto GameStateAthena = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

        if (GameStateAthena && GameStateAthena->GetPlaylistDataIsLoaded())
        {
            auto VolumeManager = GameStateAthena->GetVolumeManager();

            if (GameStateAthena->AreAllAdditionalPlaylistLevelsLoaded() && GameStateAthena->AreAllAdditionalPlaylistLevelsVisible() && (!VolumeManager || !VolumeManager->IsInSpawningStartup()))
                return CountReadyPlayers() >= GetWarmupRequiredPlayerCount();
        }
    }

    return false;
}

int32 AFortGameModeAthena::CountReadyPlayers()
{
    static auto CountReadyPlayers = Anchor::Function<int32 (*)(AFortGameModeAthena*, bool)>(TEXT("AFortGameModeAthena::CountReadyPlayers"))
        .At(DETOUR_ORIGINAL(AFortGameModeAthena, ReadyToStartMatch_Implementation))
        .Find("call 0x*")
        .Target()
        .Get();

    return CountReadyPlayers(this, false);
}

APawn* AFortGameModeAthena::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    DETOUR(AFortGameModeAthena, SpawnDefaultPawnFor_Implementation, Anchor::Hook()
        .String(TEXT("SpawnDefaultPawnFor: Failed due to no world"))
        .Start()
        .Inline());

    auto PlayerController = Cast<AFortPlayerControllerAthena>(NewPlayer);

    if (!PlayerController)
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no player controller"));
        return nullptr;
    }

    if (!StartSpot)
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no start spot"));
        return nullptr;
    }

    if (!GetDefaultPawnClassForController(NewPlayer))
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no default pawn class"));
        return nullptr;
    }

    if (!GetWorld())
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no world"));
        return nullptr;
    }

    auto Pawn = AGameModeBase::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
    auto PlayerState = Cast<AFortPlayerState>(NewPlayer->GetPlayerState());

    if (Pawn && PlayerState)
    {
        auto bInitializeGameplay = PlayerState->GetWorldPlayerId() == -1;

        if (bInitializeGameplay)
        {
            int32 WorldPlayerId = 0;

            for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
            {
                if (auto FortPlayerState = Cast<AFortPlayerState>(OtherPlayerState))
                    WorldPlayerId = FMath::Max(WorldPlayerId, FortPlayerState->GetWorldPlayerId() + 1);
            }

            PlayerState->SetWorldPlayerId(WorldPlayerId);
        }

        PlayerState->ApplyCharacterCustomization(Pawn);

        static auto AbilitySystemInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemInterface"));

        if (auto AbilitySet = UFortGameData::Get()->GetAthenaPlayerAbilitySet())
        {
            for (auto& GameplayAbility : AbilitySet->GetGameplayAbilities())
            {
                if (!PlayerState->GetAbilitySystemComponent()->FindAbilitySpecFromClass((UClass*)GameplayAbility))
                    PlayerState->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GameplayAbility.GetDefaultObject(), 1, INDEX_NONE, AbilitySet));
            }

            if (bInitializeGameplay)
            {
                TScriptInterface<IAbilitySystemInterface> AbilitySystemInterfaceActor;

                AbilitySystemInterfaceActor.SetObject(PlayerState);
                AbilitySystemInterfaceActor.SetInterface(PlayerState->GetInterfaceAddress(AbilitySystemInterface));

                UFortKismetLibrary::EquipFortAbilitySet(AbilitySystemInterfaceActor, AbilitySet, nullptr);
            }
        }

        static auto bPersistentGameplayEffects = Anchor::Exists()
            .String(TEXT("STAT_Fort_GameStateZoneApplyingPersistentEffects"))
            .Get();

        if (bInitializeGameplay && bPersistentGameplayEffects)
        {
            if (auto GameState = Cast<AFortGameStateZone>(GetWorld()->GetGameState()))
                GameState->OnAbilitySystemInterfaceReadyForPersistentGameplayEffects((IAbilitySystemInterface*)PlayerState->GetInterfaceAddress(AbilitySystemInterface));
        }

        auto CustomizationLoadout = PlayerController->GetCustomizationLoadout();

        if (auto Pickaxe = CustomizationLoadout ? CustomizationLoadout->GetPickaxe() : nullptr)
        {
            if (!PlayerController->K2_FindExistingItemForDefinition(Pickaxe->GetWeaponDefinition(), false))
                UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, Pickaxe->GetWeaponDefinition(), 1);
        }

        for (auto& StartingItem : GetStartingItems())
        {
            if (!PlayerController->K2_FindExistingItemForDefinition(StartingItem.Item, false))
                UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, Cast<UFortWorldItemDefinition>(StartingItem.Item), StartingItem.Count);
        }
    }

    return Pawn;
}

void AFortGameModeAthena::PlacePlayerOnTeam(AController* Player)
{
    DETOUR(AFortGameModeAthena, PlacePlayerOnTeam, Anchor::Hook()
        .If(FGameMemberInfoArray::StaticStruct())
        .String(TEXT("PlacePlayerOnTeam for %s Current: %d Actual: %d"))
        .Start()
        .Log(TEXT("AFortGameMode::PlacePlayerOnTeam"))
        .Caller()
        .Inline());

    Original(Player);

    auto PlayerState = Player ? Cast<AFortPlayerStateAthena>(Player->GetPlayerState()) : nullptr;
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (!PlayerState || !GameState)
        return;

    auto& GameMemberInfoArray = GameState->GetGameMemberInfoArray();
    auto Member = new(GameMemberInfoArray.GetMembers()) FGameMemberInfo();

    Member->SquadId = PlayerState->GetSquadId();
    Member->TeamIndex = PlayerState->GetTeamIndex();
    Member->MemberUniqueId = PlayerState->GetUniqueId();

    GameMemberInfoArray.MarkItemDirty(*Member);
}

void AFortGameModeAthena::AddToAlivePlayers(AFortPlayerControllerAthena* PlayerController)
{
    static auto AddToAlivePlayers = Anchor::Function<void (*)(AFortGameModeAthena*, AFortPlayerControllerAthena*)>(TEXT("AFortGameModeAthena::AddToAlivePlayers"))
        .String(TEXT("FortGameModeAthena: Player [%s] added to alive players list (Team [%d]).  Player count is now [%d].  Team count is now [%d]. "))
        .Start()
        .Get();

    AddToAlivePlayers(this, PlayerController);
}

void AFortGameModeAthena::RemoveFromAlivePlayers(AFortPlayerControllerAthena* PC, APlayerState* RemovalInstigator, APawn* FinisherPawn, UFortWeaponItemDefinition* FinishingWeapon, EDeathCause DeathCause, bool bIsTeamSwitching)
{
    static auto RemoveFromAlivePlayers = Anchor::Function<void (*)(AFortGameModeAthena*, AFortPlayerControllerAthena*, APlayerState*, APawn*, UFortWeaponItemDefinition*, EDeathCause, bool)>(TEXT("AFortGameModeAthena::RemoveFromAlivePlayers"))
        .String(TEXT("FortGameModeAthena: Player [%s] removed from alive players list*"))
        .Start()
        .Get();

    static auto bDeathCause = Anchor::Exists()
        .At((const void*)RemoveFromAlivePlayers)
        .CallSite()
        .LastSince("mov byte ptr [rsp+0x28], *", "call 0x*")
        .Get();

    static auto bTeamSwitching = Anchor::Exists()
        .At((const void*)RemoveFromAlivePlayers)
        .CallSite()
        .LastSince("mov *[rsp+0x30], *", "call 0x*")
        .Get();

    if (bTeamSwitching)
        RemoveFromAlivePlayers(this, PC, RemovalInstigator, FinisherPawn, FinishingWeapon, DeathCause, bIsTeamSwitching);
    else if (bDeathCause)
        ((void (*)(AFortGameModeAthena*, AFortPlayerControllerAthena*, APlayerState*, APawn*, UFortWeaponItemDefinition*, EDeathCause))RemoveFromAlivePlayers)(this, PC, RemovalInstigator, FinisherPawn, FinishingWeapon, DeathCause);
    else
        ((void (*)(AFortGameModeAthena*, AFortPlayerControllerAthena*, APlayerState*))RemoveFromAlivePlayers)(this, PC, RemovalInstigator);
}

void AFortGameModeAthena::OnPlaylistDataLoaded()
{
    DETOUR(AFortGameModeAthena, OnPlaylistDataLoaded, Anchor::Hook()
        .String(TEXT("PLAYLIST: Playlist Object finished loading its assets and GameMode can now use the assets AFortGameModeAthena::OnPlaylistDataLoaded(), PlaylistName is %s (Server Only)"))
        .Optional()
        .Start()
        .Inline());

    if (DETOUR_ORIGINAL(AFortGameModeAthena, OnPlaylistDataLoaded))
        Original();

    static auto bPlaylistGarbageCollection = Anchor::Exists()
        .String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f"))
        .Get();

    if (bPlaylistGarbageCollection && GetDisableGCOnServerDuringMatch())
        IConsoleManager::Get().FindConsoleVariable(TEXT("gc.TimeBetweenPurgingPendingKillObjects"))->Set(TEXT("0"), ECVF_SetByCode);

    TArray<AActor*> BuildingContainers;
    UGameplayStatics::GetAllActorsOfClass(this, ABuildingContainer::StaticClass(), BuildingContainers);

    for (auto Actor : BuildingContainers)
    {
        auto BuildingContainer = (ABuildingContainer*)Actor;

        if (BuildingContainer->GetStartAlreadySearched_Athena())
            BuildingContainer->SpawnLoot(nullptr);
    }

    if (auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState()))
    {
        if (auto MapInfo = GameState->GetMapInfo())
            MapInfo->SpawnLlamas();
    }

    if (ABuildingItemCollectorActor::StaticClass())
    {
        TArray<AActor*> ItemCollectors;
        UGameplayStatics::GetAllActorsOfClass(this, ABuildingItemCollectorActor::StaticClass(), ItemCollectors);

        for (auto ItemCollector : ItemCollectors)
            ((ABuildingItemCollectorActor*)ItemCollector)->AthenaPostApplyPlaylistDataTables();
    }

    if (ABGAConsumableSpawner::StaticClass())
    {
        TArray<AActor*> ConsumableSpawners;
        UGameplayStatics::GetAllActorsOfClass(this, ABGAConsumableSpawner::StaticClass(), ConsumableSpawners);

        for (auto ConsumableSpawner : ConsumableSpawners)
            ((ABGAConsumableSpawner*)ConsumableSpawner)->SpawnConsumables();
    }

    if (AFortAthenaVehicle::StaticClass())
        AFortAthenaVehicle::InitializeServerImplementations();
}

void AFortGameModeAthena::OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions)
{
    DETOUR(AFortGameModeAthena, OnGivenMatchAssignment, Anchor::Hook()
        .String(TEXT("Playtest-%s"))
        .Start()
        .Log(TEXT("AFortGameMode::OnGivenMatchAssignment"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameMode"))
        .LogSlot(TEXT("AFortGameMode::OnGivenMatchAssignment"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortGameModeAthena")));

    static auto Restart = THook(TEXT("AFortGameSession::Restart"), &AFortGameSession::Restart)
        .String(TEXT("FortGameModeAthena: Set AthenaGamePhase to [EndGame]."))
        .Start()
        .Find("lea rax, [0x*]")
        .Target()
        .Log(TEXT("AFortGameModeAthena::ReturnPlayersToLobby"))
        .Find("ret")
        .Last("call *")
        .Is("call 0x*")
        .Call()
        .Log(TEXT("AFortGameModeAthena::RestartGameSession"))
        .Find("lea rax, [0x*]")
        .Target()
        .SlotHere("sub rsp, *; mov rax, [rcx+0x*]; mov rcx, [rax+0x*]; test rcx, rcx; jz *; mov rax, [rcx]; call [rax+0x*]")
        .LogSlot(TEXT("TWeakBaseFunctorDelegateInstance::Execute"))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("AFortGameSession::Restart"))
        .Or()
        .String(TEXT("FortGameModeAthena: Set AthenaGamePhase to [EndGame]."))
        .Start()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("ret")
        .Last("call *")
        .Is("call 0x*")
        .Call()
        .Find("lea rax, [0x*]")
        .Target()
        .SlotHere("sub rsp, *; movzx edx, byte ptr [0x*]; lea rax, [0x*]; mov rcx, [rcx+0x*]; xor r8d, r8d; test dl, dl; cmovnz rax, r8; mov [rsp+0x*], rax; mov rcx, [rcx+0x*]; test rcx, rcx; jz *; mov rax, [rcx]; call [rax+0x*]")
        .LogSlot(TEXT("TWeakBaseFunctorDelegateInstance::Execute"))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("AFortGameSession::Restart"))
        .Or()
        .String(TEXT("FortGameModeAthena: Set AthenaGamePhase to [EndGame]."))
        .Start()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("ret")
        .Last("call *")
        .Is("call 0x*")
        .Call()
        .Find("lea rcx, [0x*]; mov rdx, *; mov [rbx], rcx")
        .Target()
        .SlotHere("mov [rsp+0x10], rbx; push rdi; sub rsp, 0x20; mov rax, [rcx+0x20]; lea rbx, [0x*]; mov dl, [0x*]")
        .LogSlot(TEXT("TWeakBaseFunctorDelegateInstance::Execute"))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("AFortGameSession::Restart"))
        .Or()
        .String(TEXT("FortGameModeAthena: Set AthenaGamePhase to [EndGame]."))
        .Start()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("ret")
        .Last("call *")
        .LogSlot(TEXT("AFortGameSession::Restart"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortGameSession"));

    static auto SpawnLoot = THook(TEXT("ABuildingContainer::SpawnLoot"), &ABuildingContainer::SpawnLoot)
        .Exec(TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer"))
        .Find("jmp [*]")
        .LogSlot(TEXT("ABuildingContainer::BounceContainer"))
        .SlotShift(TEXT("/Script/FortniteGame.Default__BuildingContainer"), sizeof(void*))
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingContainer"));

    static auto GivePickupToSlot = Anchor::Slot(TEXT("AFortPickup::GivePickupTo"))
        .String(TEXT("%s did not have PickupTargetInventoryOwnerInterface to pickup %s, already dead: %d"))
        .Start()
        .Find("jmp [*+0x*]")
        .Or()
        .FieldVirtualAny(TEXT("/Script/FortniteGame.Default__FortPickup"), ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Actor")))->FindPropertyByName(FName(TEXT("Role"))),
            "cmp byte ptr [rcx+%x], 0x3; jnz *; test rdx, rdx; jz *",
            "push rbx; sub rsp, 0x20; cmp byte ptr [rcx+%x], 0x3; mov rbx, rcx; jnz *; test rdx, rdx; jz *",
            "mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; cmp byte ptr [rcx+%x], 0x3; movzx edi, r8b; mov rbx, rcx; jnz *; test rdx, rdx; jz *",
            "mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; cmp byte ptr [rcx+%x], 0x3; mov dil, r8b; mov rbx, rcx; jnz *; test rdx, rdx; jz *")
        .Get();

    static auto PickupAthenaGivePickupTo = THook(TEXT("AFortPickupAthena::GivePickupTo"), &AFortPickup::GivePickupTo)
        .UseSlot(TEXT("/Script/FortniteGame.Default__FortPickupAthena"), GivePickupToSlot)
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPickupAthena"));

    static auto GivePickupTo = AFortPickup::GameGivePickupTo = THook(TEXT("AFortPickup::GivePickupTo"), &AFortPickup::GivePickupTo)
        .UseSlot(TEXT("/Script/FortniteGame.Default__FortPickup"), GivePickupToSlot)
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPickup"));

    static auto ServerHandlePickup = THook(TEXT("AFortPlayerPawnAthena::ServerHandlePickup_Implementation"), &AFortPlayerPawnAthena::ServerHandlePickup_Implementation)
        .If(!StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickupInfo")))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickup"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerPawn::ServerHandlePickup_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

    static auto ServerHandlePickupWithSwap = THook(TEXT("AFortPlayerPawnAthena::ServerHandlePickupWithSwap_Implementation"), &AFortPlayerPawnAthena::ServerHandlePickupWithSwap_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickupWithSwap"))
        .Optional()
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerPawn::ServerHandlePickupWithSwap_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

    static auto ServerHandlePickupWithRequestedSwap = THook(TEXT("AFortPlayerPawnAthena::ServerHandlePickupWithRequestedSwap_Implementation"), &AFortPlayerPawnAthena::ServerHandlePickupWithRequestedSwap_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickupWithRequestedSwap"))
        .Optional()
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerPawn::ServerHandlePickupWithRequestedSwap_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

    static auto ServerHandlePickupInfo = THook(TEXT("AFortPlayerPawnAthena::ServerHandlePickupInfo_Implementation"), &AFortPlayerPawnAthena::ServerHandlePickupInfo_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickupInfo"))
        .Optional()
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerPawn::ServerHandlePickupInfo_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

    static auto ServerReviveFromDBNO = THook(TEXT("AFortPlayerPawnAthena::ServerReviveFromDBNO_Implementation"), &AFortPlayerPawnAthena::ServerReviveFromDBNO_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerReviveFromDBNO"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerPawn::ServerReviveFromDBNO_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

    static auto ServerSendZiplineState = THook(TEXT("AFortPlayerPawn::ServerSendZiplineState_Implementation"), &AFortPlayerPawn::ServerSendZiplineState_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerSendZiplineState"))
        .Optional()
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerPawn::ServerSendZiplineState_Implementation"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

    static auto AttemptSpawnResources = THook(TEXT("ABuildingSMActor::AttemptSpawnResources"), &ABuildingSMActor::AttemptSpawnResources)
        .String("OwnedBuildingDamaged")
        .Find("call [*+0x*]", 2)
        .LogSlot(TEXT("ABuildingSMActor::AttemptSpawnResources"))
        .InChildVTablesCounted(ABuildingSMActor::StaticClass());

    static auto SetupCachedHostVehicleRepped = THook(TEXT("IFortMountedWeaponInterface::SetupCachedHostVehicleRepped"), &IFortMountedWeaponInterface::SetupCachedHostVehicleRepped)
        .String(TEXT("DebugVehicleWeaponAttachment: AFortWeaponRangedForVehicle::AttachWeaponTo(Actor:%s(Role: %s))  Failed to set up MountedWeaponInfoRepped.GetHostVehicleCached()!"))
        .Optional()
        .Start()
        .Find("lea rcx, [*+0x*]; mov rdx, *; mov rax, [rcx]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("IFortMountedWeaponInterface::SetupCachedHostVehicleRepped"))
        .InInterfaceVTables(AFortWeapon::StaticClass(), (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMountedWeaponInterface")), "ret*");

    static auto BeginPlay = THook(TEXT("ABuildingContainer::BeginPlay"), &ABuildingContainer::BeginPlay)
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Log(TEXT("ENGINE_ReceiveBeginPlay"))
        .Reference("mov rdx, *")
        .Start()
        .Log(TEXT("AActor::ReceiveBeginPlay"))
        .Caller()
        .Log(TEXT("AActor::BeginPlay"))
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .LogSlot(TEXT("AActor::BeginPlay"))
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingContainer"));

    static auto bTakesSucceeded = Anchor::Exists()
        .At(DETOUR_ORIGINAL(AFortGameModeAthena, OnGivenMatchAssignment))
        .Is("test dl, dl")
        .Get();

    static auto bStripped = Anchor::Exists()
        .At(DETOUR_ORIGINAL(AFortGameModeAthena, OnGivenMatchAssignment))
        .Is("ret*")
        .Get();

    if (bStripped)
    {
        if (auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState()))
            GameState->SetCurrentPlaylistName(GetCurrentPlaylistName());
    }
    else if (bTakesSucceeded)
        ((void (*)(AFortGameModeAthena*, bool, const TMap<FString, FString>&))DETOUR_ORIGINAL(AFortGameModeAthena, OnGivenMatchAssignment))(this, true, MatchOptions);
    else
        Original(MatchOptions);

    if (!DETOUR_ORIGINAL(AFortGameModeAthena, OnPlaylistDataLoaded))
        OnPlaylistDataLoaded();
}

TSubclassOf<AActor> AFortGameModeAthena::GetVehicleClassOverride(TSubclassOf<AActor> DefaultVehicleClass) const
{
    static auto GetVehicleClassOverride = Anchor::Function<TSubclassOf<AActor>* (*)(const AFortGameModeAthena*, TSubclassOf<AActor>*, TSubclassOf<AActor>)>(TEXT("AFortGameModeAthena::GetVehicleClassOverride"))
        .Native(TEXT("/Script/FortniteGame.FortGameModeAthena:GetVehicleClassOverride"))
        .Get();

    TSubclassOf<AActor> Result;

    return *GetVehicleClassOverride(this, &Result, DefaultVehicleClass);
}

bool AFortGameModeAthena::ShouldSpawnVehicle(TSubclassOf<AFortAthenaVehicle> VehicleClass)
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (!GameState)
        return false;

    if (!GameState->GetPlaylistDataIsLoaded())
        return true;

    auto MapInfo = GameState->GetMapInfo();

    if (!MapInfo)
        return false;

    for (auto& VehicleClassDetails : MapInfo->GetVehicleClassDetails())
    {
        if (VehicleClassDetails.GetVehicleClass().IsNull())
            continue;

        if (VehicleClassDetails.GetVehicleClass().LoadSynchronous() == *VehicleClass)
        {
            const float MinSpawnPercent = FMath::Clamp(VehicleClassDetails.GetVehicleMinSpawnPercent().GetValueAtLevel(0.0f) * 0.01f, 0.0f, 1.0f);
            const float MaxSpawnPercent = FMath::Clamp(VehicleClassDetails.GetVehicleMaxSpawnPercent().GetValueAtLevel(0.0f) * 0.01f, 0.0f, 1.0f);
            const float SpawnPercent = FMath::Lerp(MinSpawnPercent, MaxSpawnPercent, FMath::FRand());

            return SpawnPercent >= FMath::FRand();
        }
    }

    return false;
}

bool AFortGameModeAthena::IsMapInfoInitialized() const
{
    static auto bMapInfoInitialized = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bMapInfoInitialized")));

    return bMapInfoInitialized->GetPropertyValue_InContainer(this);
}

void AFortGameModeAthena::SetSafeZoneIndicatorShrinkTimes()
{
    DETOUR(AFortGameModeAthena, SetSafeZoneIndicatorShrinkTimes, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortSafeZoneIndicator, OffsetShrinkTimesFromServerTime) && !DETOUR_ORIGINAL(AFortSafeZoneIndicator, SetShrinkDuration) && AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("call 0x*; mov rcx, [*]; mov *, [rcx+0x*]")
        .Target()
        .Optional()
        .Inline());

    static auto WaitTimes = Anchor::Offset(TEXT("FFortSafeZoneDefinition wait times"))
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("call 0x*; mov rcx, [*]; mov *, [rcx+0x*]")
        .Target()
        .Find("cmp eax, [rdi+0x*]; jnl 0x*; movsxd rcx, eax; mov rax, [rdi+0x*]; movss xmm0, dword ptr [rax+rcx*4]")
        .Find("mov rax, [rdi+0x*]")
        .Disp();

    Original();

    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;
    auto SafeZoneIndicator = GetSafeZoneIndicator();
    static auto SafeZoneDefinition = MapInfo ? MapInfo->GetClass()->FindPropertyByName(FName(TEXT("SafeZoneDefinition"))) : nullptr;

    if (!MapInfo || !SafeZoneIndicator || !SafeZoneDefinition || WaitTimes <= 0)
        return;

    auto Definition = SafeZoneDefinition->ContainerPtrToValuePtr<uint8>(MapInfo);
    auto& SafeZoneWaitTimes = *(TArray<float>*)(Definition + WaitTimes);
    auto& SafeZoneShrinkTimes = *(TArray<float>*)(Definition + WaitTimes + sizeof(TArray<float>));
    const int32 SafeZonePhase = GetSafeZonePhase();
    const float WaitTime = SafeZonePhase >= 0 && SafeZonePhase < SafeZoneWaitTimes.Num() ? SafeZoneWaitTimes[SafeZonePhase] : 0.0f;

    if (WaitTime < 0.0f)
        return;

    SafeZoneIndicator->OffsetShrinkTimesFromServerTime(WaitTime);
    SafeZoneIndicator->SetShrinkDuration(SafeZonePhase >= 0 && SafeZonePhase < SafeZoneShrinkTimes.Num() ? SafeZoneShrinkTimes[SafeZonePhase] : 0.0f);
}

void AFortGameModeAthena::StartNewSafeZonePhase(int32 NewSafeZonePhase)
{
    DETOUR(AFortGameModeAthena, StartNewSafeZonePhase, Anchor::Hook()
        .If(AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("FutureReplicator"))) != nullptr)
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Start()
        .Optional()
        .Inline());

    Original(NewSafeZonePhase);

    auto SafeZoneIndicator = GetSafeZoneIndicator();
    auto FutureReplicator = SafeZoneIndicator ? SafeZoneIndicator->GetFutureReplicator() : nullptr;
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;
    auto SafeZoneRadius = MapInfo ? MapInfo->GetSafeZoneRadius() : nullptr;
    auto& SafeZoneLocations = GetSafeZoneLocations();

    if (!FutureReplicator || !SafeZoneRadius || !SafeZoneLocations.Num())
        return;

    const int32 NextSafeZoneIndex = NewSafeZonePhase < 0 ? GetSafeZonePhase() : GetSafeZonePhase() + 1;
    const int32 NextNextSafeZoneIndex = FMath::Clamp(NextSafeZoneIndex + 1, 0, SafeZoneLocations.Num() - 1);

    SafeZoneIndicator->GetNextNextCenter() = SafeZoneLocations[NextNextSafeZoneIndex];
    SafeZoneIndicator->GetNextNextRadius() = SafeZoneRadius->GetValueAtLevel((float)NextNextSafeZoneIndex);

    FutureReplicator->GetNextNextCenter() = SafeZoneIndicator->GetNextNextCenter();
    FutureReplicator->GetNextNextRadius() = SafeZoneIndicator->GetNextNextRadius();
    FutureReplicator->ForceNetUpdate();
}

int32 AFortGameModeAthena::GetSafeZonePhase() const
{
    static auto SafeZonePhase = GetClass()->FindPropertyByName(FName(TEXT("SafeZonePhase")));

    return *SafeZonePhase->ContainerPtrToValuePtr<int32>(this);
}

AFortSafeZoneIndicator* AFortGameModeAthena::GetSafeZoneIndicator() const
{
    static auto SafeZoneIndicator = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneIndicator")));

    return *SafeZoneIndicator->ContainerPtrToValuePtr<AFortSafeZoneIndicator*>(this);
}

TArray<FVector>& AFortGameModeAthena::GetSafeZoneLocations()
{
    static auto SafeZoneLocations = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneLocations")));

    return *SafeZoneLocations->ContainerPtrToValuePtr<TArray<FVector>>(this);
}

void AFortGameModeAthena::OverrideAISettings(UAthenaAISettings* InAISettings)
{
    GetAISettings() = InAISettings;

    auto World = GetWorld();
    auto AISystem = World ? Cast<UAthenaAISystem>(World->GetAISystem()) : nullptr;

    if (auto PerceptionManager = AISystem ? AISystem->GetPerceptionManager() : nullptr)
        PerceptionManager->OnAISettingsUpdated(InAISettings);

    GetAIDirector();
}

UAthenaAISettings*& AFortGameModeAthena::GetAISettings()
{
    static auto AISettings = GetClass()->FindPropertyByName(FName(TEXT("AISettings")));

    return *AISettings->ContainerPtrToValuePtr<UAthenaAISettings*>(this);
}

UFortServerBotManagerAthena*& AFortGameModeAthena::GetServerBotManager()
{
    static auto ServerBotManager = GetClass()->FindPropertyByName(FName(TEXT("ServerBotManager")));

    return *ServerBotManager->ContainerPtrToValuePtr<UFortServerBotManagerAthena*>(this);
}

TSubclassOf<UFortServerBotManagerAthena> AFortGameModeAthena::GetServerBotManagerClass() const
{
    static auto ServerBotManagerClass = GetClass()->FindPropertyByName(FName(TEXT("ServerBotManagerClass")));

    return *ServerBotManagerClass->ContainerPtrToValuePtr<TSubclassOf<UFortServerBotManagerAthena>>(this);
}

bool AFortGameModeAthena::GetDisableGCOnServerDuringMatch() const
{
    static auto bDisableGCOnServerDuringMatch = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDisableGCOnServerDuringMatch")));

    return bDisableGCOnServerDuringMatch->GetPropertyValue_InContainer(this);
}

int32& AFortGameModeAthena::GetWarmupRequiredPlayerCount()
{
    static auto WarmupRequiredPlayerCount = GetClass()->FindPropertyByName(FName(TEXT("WarmupRequiredPlayerCount")));

    return *WarmupRequiredPlayerCount->ContainerPtrToValuePtr<int32>(this);
}

TArray<AFortPlayerControllerAthena*>& AFortGameModeAthena::GetAlivePlayers()
{
    static auto AlivePlayers = GetClass()->FindPropertyByName(FName(TEXT("AlivePlayers")));

    return *AlivePlayers->ContainerPtrToValuePtr<TArray<AFortPlayerControllerAthena*>>(this);
}

TArray<AFortAthenaAIBotController*>& AFortGameModeAthena::GetAliveBots()
{
    static auto AliveBots = GetClass()->FindPropertyByName(FName(TEXT("AliveBots")));

    return *AliveBots->ContainerPtrToValuePtr<TArray<AFortAthenaAIBotController*>>(this);
}

FName AFortGameModeAthena::RedirectLootTierGroup(FName LootTierGroup) const
{
    static auto RedirectAthenaLootTierGroups = GetClass()->FindPropertyByName(FName(TEXT("RedirectAthenaLootTierGroups")));
    static auto SupportedAthenaLootTierGroups = GetClass()->FindPropertyByName(FName(TEXT("SupportedAthenaLootTierGroups")));
    static auto RedirectLootTierGroup = Anchor::Optional<FName* (*)(const AFortGameModeAthena*, FName*, FName)>(TEXT("AFortGameModeAthena::RedirectLootTierGroup"))
        .FieldUserOf(this, "mov *, [*+%x]", RedirectAthenaLootTierGroups)
        .Or()
        .If(!RedirectAthenaLootTierGroups)
        .NameUserOf(this, "Loot_AthenaSupplyDrop")
        .Get();

    FName Result(LootTierGroup);

    if (RedirectLootTierGroup)
    {
        RedirectLootTierGroup(this, &Result, LootTierGroup);

        return Result;
    }

    if (!RedirectAthenaLootTierGroups || !SupportedAthenaLootTierGroups)
        return Result;

    for (auto& Redirect : *RedirectAthenaLootTierGroups->ContainerPtrToValuePtr<TMap<FName, FName>>(this))
    {
        if (Redirect.Key == LootTierGroup)
            return Redirect.Value;
    }

    for (auto& SupportedGroup : *SupportedAthenaLootTierGroups->ContainerPtrToValuePtr<TArray<FName>>(this))
    {
        if (SupportedGroup == LootTierGroup)
            return LootTierGroup;
    }

    return FName(NAME_None);
}

void AFortGameModeAthena::PerformAsyncLoadForGameFeatureLootData()
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto GameFeaturePluginManager = UFortGameFeaturePluginManager::Get();
    auto GameFeaturesSubsystem = GameFeaturePluginManager ? nullptr : UGameFeaturesSubsystem::Get();

    if (!GameState || (!GameFeaturePluginManager && !GameFeaturesSubsystem))
        return;

    TArray<UFortGameFeatureData*> GameFeatureDatas;

    if (GameFeaturePluginManager)
    {
        GameFeaturePluginManager->GetGameFeatureDataForActivePlugins(GameFeatureDatas);
    }
    else
    {
        TArray<const UGameFeatureData*> ActivePluginFeatureDatas;
        GameFeaturesSubsystem->GetGameFeatureDataForActivePlugins(ActivePluginFeatureDatas);

        for (auto GameFeatureData : ActivePluginFeatureDatas)
        {
            if (auto FortGameFeatureData = Cast<UFortGameFeatureData>((UObject*)GameFeatureData))
                GameFeatureDatas.Add(FortGameFeatureData);
        }
    }

    if (!GameFeatureDatas.Num())
        return;

    auto PlaylistContextTags = GameState->GetAthenaPlaylistContextTags();

    for (auto GameFeatureData : GameFeatureDatas)
    {
        if (!GameFeatureData)
            continue;

        GameFeatureData->GetLootTierDataForPlaylist(PlaylistContextTags, GetGameFeatureLootTierData());
        GameFeatureData->GetLootPackageDataForPlaylist(PlaylistContextTags, GetGameFeatureLootPackageData());
    }

    for (auto& LootTierData : GetGameFeatureLootTierData())
        LootTierData.LoadSynchronous();

    for (auto& LootPackageData : GetGameFeatureLootPackageData())
        LootPackageData.LoadSynchronous();
}

TArray<TSoftObjectPtr<UDataTable>>& AFortGameModeAthena::GetGameFeatureLootTierData()
{
    static auto GameFeatureLootTierData = StaticClass()->FindPropertyByName(FName(TEXT("GameFeatureLootTierData")));

    return *GameFeatureLootTierData->ContainerPtrToValuePtr<TArray<TSoftObjectPtr<UDataTable>>>(this);
}

TArray<TSoftObjectPtr<UDataTable>>& AFortGameModeAthena::GetGameFeatureLootPackageData()
{
    static auto GameFeatureLootPackageData = StaticClass()->FindPropertyByName(FName(TEXT("GameFeatureLootPackageData")));

    return *GameFeatureLootPackageData->ContainerPtrToValuePtr<TArray<TSoftObjectPtr<UDataTable>>>(this);
}
