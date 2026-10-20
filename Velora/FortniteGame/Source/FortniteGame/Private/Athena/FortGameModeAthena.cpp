#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "FortGameStateZone.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/AI/AthenaAISystem.h"
#include "Athena/AI/AthenaAIPerceptionManager.h"
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
    static auto CountReadyPlayers = Anchor::Function<int32 (*)(AFortGameModeAthena*)>(TEXT("AFortGameModeAthena::CountReadyPlayers"))
        .At(DETOUR_ORIGINAL(AFortGameModeAthena, ReadyToStartMatch_Implementation))
        .Find("call 0x*")
        .Target()
        .Get();

    return CountReadyPlayers(this);
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

            PlayerState->GetWorldPlayerId() = WorldPlayerId;
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

        static auto bPersistentGameplayEffects = Scanner::String(TEXT("STAT_Fort_GameStateZoneApplyingPersistentEffects")).Get() != nullptr;

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

void AFortGameModeAthena::OnPlaylistDataLoaded()
{
    DETOUR(AFortGameModeAthena, OnPlaylistDataLoaded, Anchor::Hook()
        .String(TEXT("PLAYLIST: Playlist Object finished loading its assets and GameMode can now use the assets AFortGameModeAthena::OnPlaylistDataLoaded(), PlaylistName is %s (Server Only)"))
        .Optional()
        .Start()
        .Inline());

    if (DETOUR_ORIGINAL(AFortGameModeAthena, OnPlaylistDataLoaded))
        Original();

    static auto bPlaylistGarbageCollection = Scanner::String(TEXT("PLAYLIST: Garbage Collection is being overridden by Playlist: %s, and is setting it to %f")).Get() != nullptr;

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

    static auto Restart = []
    {
        auto ReturnPlayersToLobby = Scanner::String(TEXT("FortGameModeAthena: Set AthenaGamePhase to [EndGame].")).Start().Find("lea rax, [0x*]").Target(TEXT("AFortGameModeAthena::ReturnPlayersToLobby"));
        auto Call = Scanner::At(ReturnPlayersToLobby).Find("ret").Last("call *");

        if (Call.Is("call 0x*"))
        {
            auto Ticker = (const void* const*)Scanner::At(Call.Call().Get(TEXT("AFortGameModeAthena::RestartGameSession"))).Find("lea rax, [0x*]").Target();
            auto Execute = Ticker ? Scanner::Slot(TEXT("TWeakBaseFunctorDelegateInstance::Execute"), Ticker, [](const Scanner::FCode& Entry) { return Entry.Is("sub rsp, *; mov rax, [rcx+0x*]; mov rcx, [rax+0x*]; test rcx, rcx; jz *; mov rax, [rcx]; call [rax+0x*]"); }) : 0;

            Call = Scanner::At(Scanner::IsSlot(Execute) ? Ticker[Execute / sizeof(void*)] : nullptr).Find("call [rax+0x*]");
        }

        auto VTable = *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameSession"));

        return DetourVTable(TEXT("AFortGameSession::Restart"), VTable, Call.Slot(TEXT("AFortGameSession::Restart")), &AFortGameSession::Restart);
    }();

    static auto SpawnLoot = []
    {
        auto BounceContainer = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer")).Find("jmp [*]").Slot(TEXT("ABuildingContainer::BounceContainer"));
        auto VTable = *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingContainer"));

        return DetourVTable(TEXT("ABuildingContainer::SpawnLoot"), VTable, Scanner::IsSlot(BounceContainer) ? BounceContainer + (int32)sizeof(void*) : 0, &ABuildingContainer::SpawnLoot);
    }();

    static auto GivePickupTo = []
    {
        auto VTable = *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickup"));
        auto GivePickupTo = Scanner::String(TEXT("%s did not have PickupTargetInventoryOwnerInterface to pickup %s, already dead: %d")).Start().Find("jmp [*+0x*]").Disp();

        if (!GivePickupTo)
        {
            auto Role = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Actor")))->FindPropertyByName(FName(TEXT("Role")))->GetOffset_ForInternal();
            auto Body = std::format("cmp byte ptr [rcx+{:#x}], 0x3; jnz *; test rdx, rdx; jz *", Role);
            auto FrameBody = std::format("push rbx; sub rsp, 0x20; cmp byte ptr [rcx+{:#x}], 0x3; mov rbx, rcx; jnz *; test rdx, rdx; jz *", Role);
            auto DestoryAfterPickupBody = std::format("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; cmp byte ptr [rcx+{:#x}], 0x3; movzx edi, r8b; mov rbx, rcx; jnz *; test rdx, rdx; jz *", Role);

            GivePickupTo = Scanner::Slot(TEXT("AFortPickup::GivePickupTo"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Is(Body) || Entry.Is(FrameBody) || Entry.Is(DestoryAfterPickupBody); });
        }
        else
        {
            GivePickupTo = Scanner::Slot(TEXT("AFortPickup::GivePickupTo"), GivePickupTo);
        }

        auto AthenaVTable = *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickupAthena"));

        DetourVTable(TEXT("AFortPickupAthena::GivePickupTo"), AthenaVTable, GivePickupTo, &AFortPickup::GivePickupTo);

        return AFortPickup::GameGivePickupTo = DetourVTable(TEXT("AFortPickup::GivePickupTo"), VTable, GivePickupTo, &AFortPickup::GivePickupTo);
    }();

    static auto ServerHandlePickup = THook(TEXT("AFortPlayerPawnAthena::ServerHandlePickup_Implementation"), &AFortPlayerPawnAthena::ServerHandlePickup_Implementation)
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

    static auto AttemptSpawnResources = []
    {
        auto AttemptSpawnResources = Scanner::String("OwnedBuildingDamaged").Find("call [*+0x*]", 2).Slot(TEXT("ABuildingSMActor::AttemptSpawnResources"));
        auto BuildingSMActorClass = ABuildingSMActor::StaticClass();
        auto Stub = Scanner::Virtual<const void*>(BuildingSMActorClass->GetDefaultObject(false), AttemptSpawnResources);

        auto Classes = 0;

        for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(BuildingSMActorClass))
                continue;

            if (auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false))
            {
                if (Stub && Scanner::Virtual<const void*>(DefaultObject, AttemptSpawnResources) == Stub)
                {
                    DetourVTable(nullptr, *(void***)DefaultObject, AttemptSpawnResources, &ABuildingSMActor::AttemptSpawnResources);
                    Classes++;
                }
            }
        }

        UE_LOG(LogVelora, Log, TEXT("Hooked ABuildingSMActor::AttemptSpawnResources in %d classes"), Classes);

        return AttemptSpawnResources;
    }();

    static auto SetupCachedHostVehicleRepped = []
    {
        auto AttachWeaponTo = Scanner::String(TEXT("DebugVehicleWeaponAttachment: AFortWeaponRangedForVehicle::AttachWeaponTo(Actor:%s(Role: %s))  Failed to set up MountedWeaponInfoRepped.GetHostVehicleCached()!")).Start();

        if (!AttachWeaponTo.Get())
            return 0;

        auto SetupCachedHostVehicleRepped = AttachWeaponTo.Find("lea rcx, [*+0x*]; mov rdx, *; mov rax, [rcx]; call [rax+0x*]").Find("call [rax+0x*]").Slot(TEXT("IFortMountedWeaponInterface::SetupCachedHostVehicleRepped"));
        auto FortMountedWeaponInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMountedWeaponInterface"));
        auto Classes = 0;

        for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(AFortWeapon::StaticClass()))
                continue;

            auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false);
            auto MountedWeaponInterface = DefaultObject ? DefaultObject->GetInterfaceAddress(FortMountedWeaponInterface) : nullptr;

            if (MountedWeaponInterface && Scanner::At(Scanner::Virtual<const void*>(MountedWeaponInterface, SetupCachedHostVehicleRepped)).Is("ret*"))
            {
                DetourVTable(nullptr, *(void***)MountedWeaponInterface, SetupCachedHostVehicleRepped, &IFortMountedWeaponInterface::SetupCachedHostVehicleRepped);
                Classes++;
            }
        }

        UE_LOG(LogVelora, Log, TEXT("Hooked IFortMountedWeaponInterface::SetupCachedHostVehicleRepped in %d classes"), Classes);

        return SetupCachedHostVehicleRepped;
    }();

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

    static auto bTakesSucceeded = Scanner::At(DETOUR_ORIGINAL(AFortGameModeAthena, OnGivenMatchAssignment)).Is("test dl, dl");
    static auto bStripped = Scanner::At(DETOUR_ORIGINAL(AFortGameModeAthena, OnGivenMatchAssignment)).Is("ret*");

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

FName AFortGameModeAthena::RedirectLootTierGroup(FName LootTierGroup) const
{
    static auto RedirectLootTierGroup = Scanner::Function<FName* (*)(const AFortGameModeAthena*, FName*, FName)>([this]
    {
        auto VTable = *(const uint8_t***)this;
        auto Image = Scanner::Base();
        auto ImageEnd = Image + ((IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew))->OptionalHeader.SizeOfImage;
        auto RedirectAthenaLootTierGroups = GetClass()->FindPropertyByName(FName(TEXT("RedirectAthenaLootTierGroups")));
        auto RedirectMap = RedirectAthenaLootTierGroups ? std::format("mov *, [*+{:#x}]", RedirectAthenaLootTierGroups->GetOffset_ForInternal()) : std::string();

        for (int32 Index = 0; Scanner::IsCode(VTable[Index]); Index++)
        {
            if (VTable[Index] < Image || VTable[Index] >= ImageEnd)
                continue;

            if (RedirectAthenaLootTierGroups)
            {
                if (Scanner::At(VTable[Index]).Has(RedirectMap))
                    return Scanner::At(VTable[Index]).Get(TEXT("AFortGameModeAthena::RedirectLootTierGroup"));

                continue;
            }

            for (auto Name = Scanner::At(VTable[Index]).Find("lea rdx, [0x*]"); Name.Get(); Name = Name.Find("lea rdx, [0x*]"))
            {
                auto Target = Name.Target();

                if (Target >= Image && Target + sizeof("Loot_AthenaSupplyDrop") <= ImageEnd && !memcmp(Target, "Loot_AthenaSupplyDrop", sizeof("Loot_AthenaSupplyDrop")))
                    return Scanner::At(VTable[Index]).Get(TEXT("AFortGameModeAthena::RedirectLootTierGroup"));
            }
        }

        return Scanner::At(nullptr).Get(TEXT("AFortGameModeAthena::RedirectLootTierGroup"));
    }());

    FName Result(LootTierGroup);

    RedirectLootTierGroup(this, &Result, LootTierGroup);

    return Result;
}
