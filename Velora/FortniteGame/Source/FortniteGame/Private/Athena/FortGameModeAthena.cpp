#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Online/FortGameSession.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Building/BuildingContainer.h"
#include "Building/BGAConsumableSpawner.h"
#include "Building/BuildingItemCollectorActor.h"
#include "Building/BuildingSMActor.h"
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

static auto SpawnDefaultPawnFor = Detour(TEXT("AFortGameModeAthena::SpawnDefaultPawnFor_Implementation"), Scanner::String(TEXT("SpawnDefaultPawnFor: Failed due to no world")).Start().Get(), &AFortGameModeAthena::SpawnDefaultPawnFor_Implementation);

static auto OnGivenMatchAssignment = Detour(TEXT("AFortGameModeAthena::OnGivenMatchAssignment"), []
{
    auto Option = Scanner::String(TEXT("match.option.warmupRequiredPlayerCount"));

    return (Option.Get() ? Option : Scanner::String(TEXT("AFortGameModeAthena::OnGivenMatchAssignment: MaxPlayerCount: %i, NumTeams: %i, WarmupRequiredPlayerCount: %i, Timeout: %.2f"))).Start().Get();
}(), &AFortGameModeAthena::OnGivenMatchAssignment);

static auto OnPlaylistDataLoaded = []
{
    auto PlaylistDataLoaded = Scanner::String(TEXT("PLAYLIST: Playlist Object finished loading its assets and GameMode can now use the assets AFortGameModeAthena::OnPlaylistDataLoaded(), PlaylistName is %s (Server Only)"));

    return PlaylistDataLoaded.Get() ? Detour(TEXT("AFortGameModeAthena::OnPlaylistDataLoaded"), PlaylistDataLoaded.Start().Get(), &AFortGameModeAthena::OnPlaylistDataLoaded) : nullptr;
}();

UClass* AFortGameModeAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));

    return Class;
}

APawn* AFortGameModeAthena::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
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
        if (PlayerState->GetWorldPlayerId() == -1)
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

        if (auto AbilitySet = UFortGameData::Get()->GetGenericPlayerAbilitySet())
        {
            for (auto& GameplayAbility : AbilitySet->GetGameplayAbilities())
            {
                if (!PlayerState->GetAbilitySystemComponent()->FindAbilitySpecFromClass((UClass*)GameplayAbility))
                    PlayerState->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GameplayAbility.GetDefaultObject(), 1, INDEX_NONE, AbilitySet));
            }
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

void AFortGameModeAthena::OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions)
{
    static auto Restart = []
    {
        auto ReturnPlayersToLobby = Scanner::String(TEXT("FortGameModeAthena: Set AthenaGamePhase to [EndGame].")).Start().Find("lea rax, [0x*]").Target(TEXT("AFortGameModeAthena::ReturnPlayersToLobby"));
        auto Restart = Scanner::At(ReturnPlayersToLobby).Find("ret").Last("call [rax+0x*]").Slot(TEXT("AFortGameSession::Restart"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameSession"));

        return DetourVTable(TEXT("AFortGameSession::Restart"), (void**)(VTable + Restart), &AFortGameSession::Restart);
    }();

    static auto SpawnLoot = []
    {
        auto BounceContainer = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer")).Find("jmp [*]").Slot(TEXT("ABuildingContainer::BounceContainer"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingContainer"));

        return DetourVTable(TEXT("ABuildingContainer::SpawnLoot"), (void**)(VTable + BounceContainer + sizeof(void*)), &ABuildingContainer::SpawnLoot);
    }();

    static auto GivePickupTo = []
    {
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickup"));
        auto GivePickupTo = Scanner::String(TEXT("%s did not have PickupTargetInventoryOwnerInterface to pickup %s, already dead: %d")).Start().Find("jmp [*+0x*]").Disp();

        if (!GivePickupTo)
        {
            auto Role = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Actor")))->FindPropertyByName(FName(TEXT("Role")))->GetOffset_ForInternal();
            auto Body = std::format("cmp byte ptr [rcx+{:#x}], 0x3; jnz *; test rdx, rdx; jz *", Role);
            auto FrameBody = std::format("push rbx; sub rsp, 0x20; cmp byte ptr [rcx+{:#x}], 0x3; mov rbx, rcx; jnz *; test rdx, rdx; jz *", Role);

            for (int32 Index = 0; !GivePickupTo && Index < 512; Index++)
            {
                if (Scanner::At(((void**)VTable)[Index]).Is(Body) || Scanner::At(((void**)VTable)[Index]).Is(FrameBody))
                    GivePickupTo = Index * (int32)sizeof(void*);
            }
        }

        GivePickupTo = Scanner::Slot(TEXT("AFortPickup::GivePickupTo"), GivePickupTo);
        auto AthenaVTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickupAthena"));

        DetourVTable(TEXT("AFortPickupAthena::GivePickupTo"), (void**)(AthenaVTable + GivePickupTo), &AFortPickup::GivePickupTo);

        return AFortPickup::GameGivePickupTo = DetourVTable(TEXT("AFortPickup::GivePickupTo"), (void**)(VTable + GivePickupTo), &AFortPickup::GivePickupTo);
    }();

    static auto ServerHandlePickup = []
    {
        auto ServerHandlePickup = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickup")).Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerPawn::ServerHandlePickup_Implementation"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

        return DetourVTable(TEXT("AFortPlayerPawnAthena::ServerHandlePickup_Implementation"), (void**)(VTable + ServerHandlePickup), &AFortPlayerPawnAthena::ServerHandlePickup_Implementation);
    }();

    static auto ServerHandlePickupWithSwap = []
    {
        auto ServerHandlePickupWithSwap = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerHandlePickupWithSwap"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

        return ServerHandlePickupWithSwap.Get() ? DetourVTable(TEXT("AFortPlayerPawnAthena::ServerHandlePickupWithSwap_Implementation"), (void**)(VTable + ServerHandlePickupWithSwap.Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerPawn::ServerHandlePickupWithSwap_Implementation"))), &AFortPlayerPawnAthena::ServerHandlePickupWithSwap_Implementation) : nullptr;
    }();

    static auto ServerReviveFromDBNO = []
    {
        auto ServerReviveFromDBNO = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerReviveFromDBNO")).Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerPawn::ServerReviveFromDBNO_Implementation"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"));

        return DetourVTable(TEXT("AFortPlayerPawnAthena::ServerReviveFromDBNO_Implementation"), (void**)(VTable + ServerReviveFromDBNO), &AFortPlayerPawnAthena::ServerReviveFromDBNO_Implementation);
    }();

    static auto AttemptSpawnResources = []
    {
        auto AttemptSpawnResources = Scanner::String("OwnedBuildingDamaged").Find("call [*+0x*]", 2).Slot(TEXT("ABuildingSMActor::AttemptSpawnResources"));
        auto BuildingSMActorClass = ABuildingSMActor::StaticClass();
        auto Stub = (*(void***)BuildingSMActorClass->GetDefaultObject(false))[AttemptSpawnResources / sizeof(void*)];

        auto Classes = 0;

        for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(BuildingSMActorClass))
                continue;

            if (auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false))
            {
                auto Slot = *(void***)DefaultObject + AttemptSpawnResources / sizeof(void*);

                if (*Slot == Stub)
                {
                    DetourVTable(nullptr, Slot, &ABuildingSMActor::AttemptSpawnResources);
                    Classes++;
                }
            }
        }

        UE_LOG(LogVelora, Log, TEXT("Hooked ABuildingSMActor::AttemptSpawnResources in %d classes"), Classes);

        return AttemptSpawnResources;
    }();

    static auto BeginPlay = []
    {
        auto ReceiveBeginPlayName = Scanner::String(TEXT("ReceiveBeginPlay")).Find("lea *, [0x*]").Target(TEXT("ENGINE_ReceiveBeginPlay"));
        auto ReceiveBeginPlay = Scanner::Reference(ReceiveBeginPlayName, "mov rdx, *").Start().Get(TEXT("AActor::ReceiveBeginPlay"));
        auto ActorBeginPlay = Scanner::CallTo(ReceiveBeginPlay).Start().Get(TEXT("AActor::BeginPlay"));
        auto ActorVTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = 0;

        while (ActorVTable[Index] != ActorBeginPlay)
            Index++;

        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingContainer"));

        return DetourVTable(TEXT("ABuildingContainer::BeginPlay"), (void**)(VTable + Index * sizeof(void*)), &ABuildingContainer::BeginPlay);
    }();

    void* Original;
    memcpy(&Original, &::OnGivenMatchAssignment, sizeof(Original));

    static auto bTakesSucceeded = Scanner::At(Original).Is("test dl, dl");

    if (bTakesSucceeded)
        ((void (*)(AFortGameModeAthena*, bool, const TMap<FString, FString>&))Original)(this, true, MatchOptions);
    else
        (this->*::OnGivenMatchAssignment)(MatchOptions);

    if (!::OnPlaylistDataLoaded)
        OnPlaylistDataLoaded();
}

void AFortGameModeAthena::OnPlaylistDataLoaded()
{
    if (::OnPlaylistDataLoaded)
        (this->*::OnPlaylistDataLoaded)();

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

        for (int32 Index = 0; VTable[Index] >= Image && VTable[Index] < ImageEnd; Index++)
        {
            for (auto Name = Scanner::At(VTable[Index]).Find("lea rdx, [0x*]"); Name.Get(); Name = Name.Find("lea rdx, [0x*]"))
            {
                auto Target = Name.Target();

                if (Target >= Image && Target + sizeof("Loot_AthenaSupplyDrop") <= ImageEnd && !memcmp(Target, "Loot_AthenaSupplyDrop", sizeof("Loot_AthenaSupplyDrop")))
                    return Scanner::At(VTable[Index]).Get(TEXT("AFortGameModeAthena::RedirectLootTierGroup"));
            }
        }

        return Scanner::At(nullptr).Get(TEXT("AFortGameModeAthena::RedirectLootTierGroup"));
    }());

    FName Result(NAME_None);

    return *RedirectLootTierGroup(this, &Result, LootTierGroup);
}
