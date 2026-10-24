#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "FortGameStateZone.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Conversation/FortConversationTaskNode_ShowFutureStormCircle.h"
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
#include "Engine/DataTable.h"
#include "Abilities/FortAbilitySet.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/FortGameplayAbility.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "FortKismetLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/EngineVersion.h"
#include "GameFramework/GameStateBase.h"
#include "Math/UnrealMathUtility.h"
#include "HAL/IConsoleManager.h"
#include "Templates/Casts.h"
#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"
#include <intrin.h>

UClass* AFortGameModeAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));

    return Class;
}

TMulticastDelegate<void, AFortGameModeAthena*>& AFortGameModeAthena::GetNotifyPlaylistDataLoaded()
{
    static auto NotifyPlaylistDataLoaded = Anchor::Data<TMulticastDelegate<void, AFortGameModeAthena*>*>(TEXT("AFortGameModeAthena::NotifyPlaylistDataLoaded"))
        .String(TEXT("PLAYLIST: Playlist Object finished loading its assets and GameMode can now use the assets AFortGameModeAthena::OnPlaylistDataLoaded(), PlaylistName is %s (Server Only)"))
        .Start()
        .Find("lea rcx, [0x*]; call 0x*; mov rdx, rdi; call 0x*")
        .Find("mov rdx, rdi; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyPlaylistDataLoaded;
}

TMulticastDelegate<void, AFortLiveSpectatorController*>& AFortGameModeAthena::GetNotifyLiveSpectatorJoined()
{
    static auto NotifyLiveSpectatorJoined = Anchor::Data<TMulticastDelegate<void, AFortLiveSpectatorController*>*>(TEXT("AFortGameModeAthena::NotifyLiveSpectatorJoined"))
        .String(TEXT("PlayerJoin,%s,%d,%d"))
        .Start()
        .Find("mov rdx, rbx; mov [rax+*], rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rax, [0x*]")
        .Target()
        .Get();

    return *NotifyLiveSpectatorJoined;
}

TMulticastDelegate<void, AFortPlayerStateAthena*, uint8>& AFortGameModeAthena::GetNotifyPlayerStateSquadAssignment()
{
    static auto NotifyPlayerStateSquadAssignment = Anchor::Data<TMulticastDelegate<void, AFortPlayerStateAthena*, uint8>*>(TEXT("AFortGameModeAthena::NotifyPlayerStateSquadAssignment"))
        .String(TEXT("PlacePlayerOnTeam for %s Current: %d Actual: %d"))
        .Start()
        .Caller()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyPlayerStateSquadAssignment;
}

TMulticastDelegate<void, TSubclassOf<AFortPawn>>& AFortGameModeAthena::GetNotifyNewAIClass()
{
    static auto NotifyNewAIClass = Anchor::Data<TMulticastDelegate<void, TSubclassOf<AFortPawn>>*>(TEXT("AFortGameModeAthena::NotifyNewAIClass"))
        .Native(TEXT("/Script/FortniteGame.FortGameModeAthena:AddAIClassToReplicationGraph"))
        .Find("mov rax, [0x*]")
        .Target()
        .Get();

    return *NotifyNewAIClass;
}

TMulticastDelegate<void, AActor*, AFortInventory*>& AFortGameModeAthena::GetNotifyAddReplicatedNonPlayerInventory()
{
    static auto NotifyAddReplicatedNonPlayerInventory = Anchor::Data<TMulticastDelegate<void, AActor*, AFortInventory*>*>(TEXT("AFortGameModeAthena::NotifyAddReplicatedNonPlayerInventory"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Entry(TEXT("/Script/CraftingRuntime.Default__CraftingObjectBGA"))
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyAddReplicatedNonPlayerInventory;
}

TMulticastDelegate<void, AActor*, AFortInventory*>& AFortGameModeAthena::GetNotifyRemoveReplicatedNonPlayerInventory()
{
    static auto NotifyRemoveReplicatedNonPlayerInventory = Anchor::Data<TMulticastDelegate<void, AActor*, AFortInventory*>*>(TEXT("AFortGameModeAthena::NotifyRemoveReplicatedNonPlayerInventory"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .SlotShift(TEXT("/Script/Engine.Default__Actor"), 0x8)
        .Entry(TEXT("/Script/CraftingRuntime.Default__CraftingObjectBGA"))
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyRemoveReplicatedNonPlayerInventory;
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

void AFortGameModeAthena::CreateAIDirector()
{
    DETOUR(AFortGameModeAthena, CreateAIDirector, Anchor::Hook()
        .If(!::Anchor::Exists()
            .String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i"))
            .Start()
            .Find("call 0x*; mov rax, [rbx+0x*]")
            .Get())
        .String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i"))
        .Optional()
        .Start()
        .Inline());

    AFortGameModeZone::CreateAIDirector();

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

    if (auto ServerBotManagerField = GetServerBotManager())
        *ServerBotManagerField = ServerBotManager;

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
        .Or()
        .String(TEXT("FortGameModeAthena::AddToAlivePlayers: Player [%s] PC [%s] added to alive players list*"))
        .Start()
        .Get();

    AddToAlivePlayers(this, PlayerController);
}

void AFortGameModeAthena::RemoveFromAlivePlayers(AFortPlayerControllerAthena* PC, APlayerState* RemovalInstigator, APawn* FinisherPawn, UFortWeaponItemDefinition* FinishingWeapon, EDeathCause DeathCause, bool bIsTeamSwitching)
{
    static auto RemoveFromAlivePlayers = Anchor::Function<void (*)(AFortGameModeAthena*, AFortPlayerControllerAthena*, APlayerState*, APawn*, UFortWeaponItemDefinition*, EDeathCause, bool)>(TEXT("AFortGameModeAthena::RemoveFromAlivePlayers"))
        .String(TEXT("FortGameModeAthena: Player [%s] removed from alive players list*"))
        .Start()
        .Or()
        .String(TEXT("FortGameModeAthena::RemoveFromAlivePlayers: Player [%s] PC [%s] removed from alive players list*"))
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

    auto GameStateAthena = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (!DETOUR_ORIGINAL(AFortGameStateAthena, OnPlaylistLootTableAssetsDataLoadCompleted) || !GameStateAthena || GameStateAthena->GetPlaylistDataIsLoaded())
    {
        TArray<AActor*> BuildingContainers;
        UGameplayStatics::GetAllActorsOfClass(this, ABuildingContainer::StaticClass(), BuildingContainers);

        for (auto Actor : BuildingContainers)
        {
            auto BuildingContainer = (ABuildingContainer*)Actor;

            if (BuildingContainer->GetStartAlreadySearched_Athena())
                BuildingContainer->SpawnLoot(nullptr);
        }
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
        .Is("call 0x*")
        .Call()
        .Find("lea r8, [rsp+0x*]; mov rdx, *; mov rcx, rax; call 0x*")
        .Call()
        .Find("lea rcx, [0x*]; mov [rbx], rcx")
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickupInfo"))
        .Find("call r*; test al, al")
        .Find("call [rax+*]")
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
        .Reference("mov ?dx, *")
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
        .Last("sar ecx, 0x1; mov [rax+0x*], ecx; mov rcx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Or()
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
        .Or()
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("sar ecx, 0x1; mov [rax+0x*], ecx; mov rcx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rax, [rdi+0x*]; movsxd rcx, ecx; movss xmm0, dword ptr [rax+rcx*4]")
        .Or()
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("sar ecx, 0x1; mov [rax+0x*], ecx; mov rcx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("cmp r15d, [rsi+0x*]; jnl 0x*; mov rax, [rsi+0x*]")
        .Find("mov rax, [rsi+0x*]")
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

    UFortConversationTaskNode_ShowFutureStormCircle::HandleSafeZoneUpdated();

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

    SafeZoneIndicator->SetNextNextCenter(TGameLayout<FVector>::Load((uint8*)SafeZoneLocations.GetData() + NextNextSafeZoneIndex * TGameLayout<FVector>::Size()));
    SafeZoneIndicator->GetNextNextRadius() = SafeZoneRadius->GetValueAtLevel((float)NextNextSafeZoneIndex);

    FutureReplicator->SetNextNextCenter(SafeZoneIndicator->GetNextNextCenter());
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

UFortServerBotManagerAthena** AFortGameModeAthena::GetServerBotManager()
{
    static auto ServerBotManager = GetClass()->FindPropertyByName(FName(TEXT("ServerBotManager")));

    return ServerBotManager ? ServerBotManager->ContainerPtrToValuePtr<UFortServerBotManagerAthena*>(this) : nullptr;
}

TSubclassOf<UFortServerBotManagerAthena> AFortGameModeAthena::GetServerBotManagerClass() const
{
    static auto ServerBotManagerClass = GetClass()->FindPropertyByName(FName(TEXT("ServerBotManagerClass")));

    return ServerBotManagerClass ? *ServerBotManagerClass->ContainerPtrToValuePtr<TSubclassOf<UFortServerBotManagerAthena>>(this) : TSubclassOf<UFortServerBotManagerAthena>();
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

    if (FName::IsOutlineNumber())
    {
        for (auto& Redirect : *RedirectAthenaLootTierGroups->ContainerPtrToValuePtr<TMap<FNameEntryId, FNameEntryId>>(this))
        {
            if (Redirect.Key == LootTierGroup.GetComparisonIndex())
                return TGameLayout<FName>::Load(&Redirect.Value);
        }
    }
    else
    {
        for (auto& Redirect : *RedirectAthenaLootTierGroups->ContainerPtrToValuePtr<TMap<FName, FName>>(this))
        {
            if (Redirect.Key == LootTierGroup)
                return Redirect.Value;
        }
    }

    auto& SupportedGroups = *SupportedAthenaLootTierGroups->ContainerPtrToValuePtr<TArray<FName>>(this);

    for (int32 GroupIndex = 0; GroupIndex < SupportedGroups.Num(); GroupIndex++)
    {
        if (TGameLayout<FName>::Load((uint8*)SupportedGroups.GetData() + GroupIndex * TGameLayout<FName>::Size()) == LootTierGroup)
            return LootTierGroup;
    }

    static auto ExternalSupportedAthenaLootTierGroups = Anchor::Data<TMap<FName, TArray<FName>>*>(TEXT("AFortGameModeAthena::ExternalSupportedAthenaLootTierGroups"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__UFortGameFeatureAction_LootTierGroups"), "lea rdx, [rcx+0x28]; mov rcx, [rcx+0x18]; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("mov r8, [0x*]")
        .Target()
        .Optional()
        .Get();

    if (ExternalSupportedAthenaLootTierGroups)
    {
        for (auto& ExternalGroups : *ExternalSupportedAthenaLootTierGroups)
        {
            if (ExternalGroups.Value.Contains(LootTierGroup))
                return LootTierGroup;
        }
    }

    return FName(NAME_None);
}

void AFortGameModeAthena::PerformAsyncLoadForGameFeatureLootData()
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (!GameState)
        return;

    TArray<UFortGameFeatureData*> GameFeatureDatas;
    UFortGameFeatureData::GetActiveGameFeatureDatas(GameFeatureDatas);

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

    if (auto OnPlaylistLootTablesAppliedDelegate = GetOnPlaylistLootTablesAppliedDelegate())
        OnPlaylistLootTablesAppliedDelegate->Broadcast();
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

FSimpleMulticastDelegate* AFortGameModeAthena::GetOnPlaylistLootTablesAppliedDelegate()
{
    static auto GameFeaturesStreamingRangedWeaponData = StaticClass()->FindPropertyByName(FName(TEXT("GameFeaturesStreamingRangedWeaponData")));
    static auto OnPlaylistLootTablesAppliedDelegate = GameFeaturesStreamingRangedWeaponData && GameFeaturesStreamingRangedWeaponData->GetOffset_ForInternal() + 0x38 <= StaticClass()->GetPropertiesSize() ? GameFeaturesStreamingRangedWeaponData->GetOffset_ForInternal() + 0x20 : 0;

    return OnPlaylistLootTablesAppliedDelegate ? (FSimpleMulticastDelegate*)((uint8*)this + OnPlaylistLootTablesAppliedDelegate) : nullptr;
}

TArray<FFortCustomRepNodeClassMapping>& AFortGameModeAthena::GetGlobalRepGraphNodeClassesToAdd()
{
    static auto GlobalRepGraphNodeClassesToAdd = StaticClass()->FindPropertyByName(FName(TEXT("GlobalRepGraphNodeClassesToAdd")));

    return *GlobalRepGraphNodeClassesToAdd->ContainerPtrToValuePtr<TArray<FFortCustomRepNodeClassMapping>>(this);
}

void AFortGameModeAthena::SetStormDamagePaused(bool bPause)
{
    DETOUR(AFortGameModeAthena, SetStormDamagePaused, Anchor::Hook()
        .If(FEngineVersion::Current().GetChangelist() == 18335626)
        .Exec(TEXT("/Script/FortniteGame.FortAthenaMutator_Storm:PauseStormDamage"))
        .Find("mov rcx, [rsi+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Optional()
        .Inline());

    UE_LOG(LogVelora, Display, TEXT("diag storm SetStormDamagePaused %d was %d caller 0x%llX time %f"), (int32)bPause, (int32)*((uint8*)this + 0xF28), (unsigned long long)((const uint8*)_ReturnAddress() - Anchor::Base()), GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0f);

    for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
    {
        static auto DiagStormMutatorClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaMutator_Storm"));
        static auto DiagSpecialEventMutatorClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaMutator_SpecialEvent"));
        auto ObjectItem = GUObjectArray.IndexToObject(Index);
        auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

        if (Object && ((DiagStormMutatorClass && Object->IsA(DiagStormMutatorClass)) || (DiagSpecialEventMutatorClass && Object->IsA(DiagSpecialEventMutatorClass))))
            UE_LOG(LogVelora, Display, TEXT("diag storm pause mutator candidate %s %s"), *GetNameSafe(Object), *GetNameSafe(Object->GetClass()));
    }

    Original(bPause);
}
