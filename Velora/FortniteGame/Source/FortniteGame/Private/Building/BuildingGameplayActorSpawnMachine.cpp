#include "pch.h"
#include "Building/BuildingGameplayActorSpawnMachine.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Pawns/FortPlayerPawn.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItemDefinition.h"
#include "FortKismetLibrary.h"
#include "GameplayEffectTypes.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Delegates/MulticastDelegateBase.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptInterface.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

struct BGA_Athena_SCMachine_C_eventTriggerRespawnFX_Parms
{
    AFortPlayerPawn* PhaseInPawn;
};

struct FPendingResurrection
{
    TWeakObjectPtr<ABuildingGameplayActorSpawnMachine> SpawnMachine;
    TWeakObjectPtr<AFortPlayerControllerAthena> PlayerController;
};

struct FPendingNextPlayer
{
    TWeakObjectPtr<ABuildingGameplayActorSpawnMachine> SpawnMachine;
    float Time;
};

struct FPendingSkydiveCheck
{
    TWeakObjectPtr<ABuildingGameplayActorSpawnMachine> SpawnMachine;
    TWeakObjectPtr<AFortPlayerPawn> Player;
    float Time;
};

static TArray<FPendingResurrection> PendingResurrections;

static TArray<FPendingNextPlayer> PendingNextPlayers;

static TArray<FPendingSkydiveCheck> PendingSkydiveChecks;

UClass* ABuildingGameplayActorSpawnMachine::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingGameplayActorSpawnMachine"));

    return Class;
}

void ABuildingGameplayActorSpawnMachine::HandleOnResurrectTimerComplete()
{
    DETOUR(ABuildingGameplayActorSpawnMachine, HandleOnResurrectTimerComplete, Anchor::Hook()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("lea rax, [0x*]; xor esi, esi")
        .IsTarget("ret*")
        .Optional()
        .AtReference());

    static auto GetGameAccountId = Anchor::Function<FUniqueNetIdRepl* (*)(const AFortPlayerController*, FUniqueNetIdRepl*)>(TEXT("AFortPlayerController::GetGameAccountId"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:GetGameAccountId"))
        .Get();
    static auto Contains = Anchor::Function<bool (*)(const TArray<FUniqueNetIdRepl>*, const FUniqueNetIdRepl&)>(TEXT("TArray<FUniqueNetIdRepl>::Contains"))
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("mov rax, [rdi]; mov rcx, rdi; call [rax+0x*]; test al, al")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("mov rax, [rsi]; mov rcx, rsi; call [rax+0x*]; test al, al")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("mov rax, [r14]; mov rcx, r14; call [rax+0x*]; test al, al")
        .Find("call 0x*", 2)
        .Target()
        .Get();

    if (!HasAuthority() || GetSpawnMachineState() != ESpawnMachineState::Active)
        return;

    for (auto PlayerState : GetWorld()->GetGameState()->GetPlayerArray())
    {
        auto PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerState);
        auto PlayerController = PlayerStateAthena ? Cast<AFortPlayerControllerAthena>(PlayerStateAthena->GetOwner()) : nullptr;

        if (!PlayerController || PlayerStateAthena->GetSquadId() != GetSquadId() || PlayerController->IsPlayerAlive())
            continue;

        bool bResurrecting = false;

        for (auto& PendingResurrection : PendingResurrections)
            bResurrecting |= PendingResurrection.PlayerController.Get() == PlayerController;

        if (bResurrecting)
            continue;

        FUniqueNetIdRepl GameAccountId;
        GetGameAccountId(PlayerController, &GameAccountId);

        if (!Contains(&GetPlayerIdsForResurrection(), GameAccountId))
            continue;

        PlayerController->RespawnPlayerAfterDeath(RebootPlayersInAir());

        PendingResurrections.Add(FPendingResurrection{ this, PlayerController });
        PendingNextPlayers.Add(FPendingNextPlayer{ this, GetWorld()->GetTimeSeconds() + GetResurrectionNextPlayerDelay().GetValueAtLevel(0.0f) });

        return;
    }

    static auto RemoveAll = Anchor::Function<void (*)(FMulticastDelegateBase<FWeakObjectPtr>*, const void*)>(TEXT("FMulticastDelegateBase::RemoveAll"))
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("lea rcx, [0x*]; mov [rsp+0x*], rcx")
        .Target()
        .Log(TEXT("ABuildingGameplayActorSpawnMachine::HandleOnTeamWipedOut"))
        .Find("lea rcx, [rdi+0x*]")
        .Call()
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("lea rcx, [0x*]; mov [rsp+0x*], rcx")
        .Target()
        .Find("lea rcx, [rbx+0x*]; mov rdx, rdi; call 0x*")
        .Call()
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("lea rax, [0x*]; mov r8, *; mov [rsp+0x*], rax")
        .Target()
        .Find("lea rcx, [rbx+0x*]; mov rdx, rdi; call 0x*")
        .Call()
        .Get();

    static auto OnTeamWipedOut = Anchor::Offset(TEXT("ABuildingGameplayActorSpawnMachine::HandleOnTeamWipedOut"))
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("lea rcx, [0x*]; mov [rsp+0x*], rcx")
        .Target()
        .Find("lea rcx, [rdi+0x*]")
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("lea rcx, [0x*]; mov [rsp+0x*], rcx")
        .Target()
        .Find("lea rcx, [rbx+0x*]; mov rdx, rdi; call 0x*")
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("lea rax, [0x*]; mov r8, *; mov [rsp+0x*], rax")
        .Target()
        .Find("lea rcx, [rbx+0x*]; mov rdx, rdi; call 0x*")
        .DispValue();

    if (auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode()))
        RemoveAll((FMulticastDelegateBase<FWeakObjectPtr>*)((uint8_t*)GameMode + OnTeamWipedOut), this);

    OnResurrectionCompleted();
}

void ABuildingGameplayActorSpawnMachine::OnResurrectionCompleted()
{
    static auto OnResurrectionCompleted = Anchor::Function<void (*)(ABuildingGameplayActorSpawnMachine*)>(TEXT("ABuildingGameplayActorSpawnMachine::OnResurrectionCompleted"))
        .Event(TEXT("OnResurrectionCompleted"))
        .Get();

    OnResurrectionCompleted(this);
}

void ABuildingGameplayActorSpawnMachine::OnPlayerPawnResurrected(AFortPlayerPawn* PlayerPawn)
{
    static auto DelayBeforeSkydiveCheck = GetClass()->FindPropertyByName(FName(TEXT("DelayBeforeSkydiveCheck")));

    if (!PlayerPawn)
        return;

    KnockPlayerPawns(PlayerPawn);
    KnockVehicles();
    TriggerRespawnFX(PlayerPawn);
    GiveWeaponsAndResources(UFortKismetLibrary::GetFortPlayerControllerFromActor(PlayerPawn));

    if (DelayBeforeSkydiveCheck)
        PendingSkydiveChecks.Add(FPendingSkydiveCheck{ this, PlayerPawn, GetWorld()->GetTimeSeconds() + *DelayBeforeSkydiveCheck->ContainerPtrToValuePtr<float>(this) });
    else
        SkydiveIfAirborne(PlayerPawn);
}

bool ABuildingGameplayActorSpawnMachine::RebootPlayersInAir() const
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.BuildingGameplayActorSpawnMachine:RebootPlayersInAir"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto RebootInAir = Anchor::Data<const int32*>(TEXT("ABuildingGameplayActorSpawnMachine::RebootPlayersInAir value"))
            .Exec(TEXT("/Script/FortniteGame.BuildingGameplayActorSpawnMachine:RebootPlayersInAir"))
            .Find("add [*+0x20], *")
            .Find("cmp dword ptr [0x*], 0x*")
            .Target()
            .Get();

        static auto RebootInAirValue = (int32)Anchor::Offset(TEXT("ABuildingGameplayActorSpawnMachine::RebootPlayersInAir compare"))
            .Exec(TEXT("/Script/FortniteGame.BuildingGameplayActorSpawnMachine:RebootPlayersInAir"))
            .Find("add [*+0x20], *")
            .Find("cmp dword ptr [0x*], 0x*")
            .Imm();

        return RebootInAir && *RebootInAir == RebootInAirValue;
    }

    static auto RebootPlayersInAir = Anchor::Function<bool (*)(const ABuildingGameplayActorSpawnMachine*)>(TEXT("ABuildingGameplayActorSpawnMachine::RebootPlayersInAir"))
        .Native(TEXT("/Script/FortniteGame.BuildingGameplayActorSpawnMachine:RebootPlayersInAir"))
        .Get();

    return RebootPlayersInAir(this);
}

void ABuildingGameplayActorSpawnMachine::KnockPlayerPawns(AFortPlayerPawn* IgnoreKnock)
{
    TArray<AActor*> OverlappingActors;
    GetBlockingVolume()->GetOverlappingActors(OverlappingActors, AFortPlayerPawn::StaticClass());

    for (auto OverlappingActor : OverlappingActors)
    {
        auto PlayerPawn = OverlappingActor != IgnoreKnock ? Cast<AFortPlayerPawn>(OverlappingActor) : nullptr;

        if (!PlayerPawn)
            continue;

        const FVector KnockDirection = GetActorForwardVector().RotateAngleAxis(FMath::FRandRange(0.0f, 360.0f), FVector(0.0f, 0.0f, 1.0f)) * GetXYKnockAmountPlayer();

        PlayerPawn->LaunchCharacter(FVector(KnockDirection.X, KnockDirection.Y, GetZKnockAmountPlayer()), true, true);
    }
}

void ABuildingGameplayActorSpawnMachine::KnockVehicles()
{
    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));
    static auto FortCharacterVehicle = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortCharacterVehicle"));

    for (auto VehicleClass : { AFortAthenaVehicle::StaticClass(), FortCharacterVehicle })
    {
        if (!VehicleClass)
            continue;

        TArray<AActor*> OverlappingActors;
        GetBlockingVolume()->GetOverlappingActors(OverlappingActors, VehicleClass);

        for (auto OverlappingActor : OverlappingActors)
        {
            auto VehicleInterface = FortVehicleInterface ? (IFortVehicleInterface*)OverlappingActor->GetInterfaceAddress(FortVehicleInterface) : nullptr;
            const AActor* Vehicle = FortVehicleInterface ? (VehicleInterface ? VehicleInterface->GetVehicleActorPtrConst() : nullptr) : Cast<AFortAthenaVehicle>(OverlappingActor);

            if (!Vehicle)
                continue;

            const FVector KnockDirection = GetActorForwardVector().RotateAngleAxis(FMath::FRandRange(0.0f, 360.0f), FVector(0.0f, 0.0f, 1.0f)) * GetXYKnockAmountVehicle();

            UFortKismetLibrary::BroadcastAbilityImpactAtLocation(this, nullptr, const_cast<AActor*>(Vehicle), FVector(KnockDirection.X, KnockDirection.Y, GetZKnockAmountVehicle()), 777.0f, FName(NAME_None), false);
        }
    }
}

void ABuildingGameplayActorSpawnMachine::TriggerRespawnFX(AFortPlayerPawn* PhaseInPawn)
{
    static auto TriggerRespawnFX = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/Items/EnvironmentalItems/SCMachine/BGA_Athena_SCMachine.BGA_Athena_SCMachine_C:TriggerRespawnFX"));
    static auto BP_VictoryDrone_C = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Blueprints/Adventures/SpawnTeleporter/BP_VictoryDrone.BP_VictoryDrone_C"));

    BGA_Athena_SCMachine_C_eventTriggerRespawnFX_Parms Parms{ PhaseInPawn };

    if (TriggerRespawnFX)
        CallRemoteFunction(TriggerRespawnFX, &Parms, nullptr, nullptr);

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(GetPadCollision()->GetComponentLocation());
    SpawnTransform.SetRotation(GetActorRotation().Quaternion());

    auto VictoryDrone = BP_VictoryDrone_C ? UGameplayStatics::BeginDeferredActorSpawnFromClass(this, BP_VictoryDrone_C, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, nullptr) : nullptr;

    if (!VictoryDrone)
        return;

    for (auto PropertyName : { TEXT("TeleportIn"), TEXT("IsAthena"), TEXT("InLobby") })
    {
        if (auto Property = (UBoolProperty*)VictoryDrone->GetClass()->FindPropertyByName(FName(PropertyName)))
            Property->SetPropertyValue_InContainer(VictoryDrone, true);
    }

    VictoryDrone = UGameplayStatics::FinishSpawningActor(VictoryDrone, SpawnTransform);

    if (!VictoryDrone)
        return;

    VictoryDrone->SetLifeSpan(2.25f);
    VictoryDrone->K2_AttachToActor(PhaseInPawn, FName(NAME_None), EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true);
}

void ABuildingGameplayActorSpawnMachine::GiveWeaponsAndResources(AFortPlayerController* FortPC)
{
    static auto FortInventoryOwnerInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));

    TArray<FFortItemEntry> LootToDrop;

    if (!UFortKismetLibrary::PickLootDrops(&LootToDrop, GetGrantedItemsLootTier(), 0, -1))
        return;

    for (auto& ItemEntry : LootToDrop)
    {
        auto ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition());

        if (!ItemDefinition)
            continue;

        TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
        InventoryOwner.SetObject(FortPC);
        InventoryOwner.SetInterface(FortPC ? FortPC->GetInterfaceAddress(FortInventoryOwnerInterface) : nullptr);

        UFortKismetLibrary::GiveItemToInventoryOwner(InventoryOwner, ItemDefinition, ItemEntry.GetCount(), true);
    }
}

void ABuildingGameplayActorSpawnMachine::SkydiveIfAirborne(AFortPlayerPawn* Player)
{
    const FVector Start = Player->GetActorLocation();

    TArray<AActor*> ActorsToIgnore;
    FHitResult OutHit;
    const bool bHit = UKismetSystemLibrary::LineTraceSingle(this, Start, Start + FVector(0.0f, 0.0f, GetHeightCheckForSkydive()), ETraceTypeQuery::TraceTypeQuery7, false, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true, FLinearColor(1.0f, 0.0f, 0.0f, 1.0f), FLinearColor(0.0f, 1.0f, 0.0f, 1.0f), 5.0f);

    if (!bHit || OutHit.GetActor().Get() != this)
        Player->BeginSkydiving(false);
}

void ABuildingGameplayActorSpawnMachine::TickPendingResurrections()
{
    for (int32 Index = PendingResurrections.Num() - 1; Index >= 0; Index--)
    {
        auto SpawnMachine = PendingResurrections[Index].SpawnMachine.Get();
        auto PlayerController = PendingResurrections[Index].PlayerController.Get();

        if (SpawnMachine && PlayerController && !PlayerController->IsPlayerAlive())
            continue;

        PendingResurrections.RemoveAt(Index);

        if (!SpawnMachine || !PlayerController)
            continue;

        if (auto GameMode = Cast<AFortGameModeAthena>(SpawnMachine->GetWorld()->GetAuthGameMode()))
            GameMode->AddToAlivePlayers(PlayerController);

        if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()))
            PlayerState->PlayerPawnRebooted();

        SpawnMachine->OnPlayerPawnResurrected(PlayerController->GetMyFortPawn());
    }

    for (int32 Index = PendingSkydiveChecks.Num() - 1; Index >= 0; Index--)
    {
        auto SpawnMachine = PendingSkydiveChecks[Index].SpawnMachine.Get();
        auto Player = PendingSkydiveChecks[Index].Player.Get();

        if (SpawnMachine && Player && SpawnMachine->GetWorld()->GetTimeSeconds() < PendingSkydiveChecks[Index].Time)
            continue;

        PendingSkydiveChecks.RemoveAt(Index);

        if (SpawnMachine && Player)
            SpawnMachine->SkydiveIfAirborne(Player);
    }

    for (int32 Index = PendingNextPlayers.Num() - 1; Index >= 0; Index--)
    {
        auto SpawnMachine = PendingNextPlayers[Index].SpawnMachine.Get();

        if (SpawnMachine && SpawnMachine->GetWorld()->GetTimeSeconds() < PendingNextPlayers[Index].Time)
            continue;

        PendingNextPlayers.RemoveAt(Index);

        if (SpawnMachine)
            SpawnMachine->HandleOnResurrectTimerComplete();
    }
}

TArray<FUniqueNetIdRepl>& ABuildingGameplayActorSpawnMachine::GetPlayerIdsForResurrection()
{
    static auto PlayerIdsForResurrection = GetClass()->FindPropertyByName(FName(TEXT("PlayerIdsForResurrection")));

    return *PlayerIdsForResurrection->ContainerPtrToValuePtr<TArray<FUniqueNetIdRepl>>(this);
}

uint8 ABuildingGameplayActorSpawnMachine::GetSquadId() const
{
    static auto SquadId = GetClass()->FindPropertyByName(FName(TEXT("SquadId")));

    return *SquadId->ContainerPtrToValuePtr<uint8>(this);
}

ESpawnMachineState ABuildingGameplayActorSpawnMachine::GetSpawnMachineState() const
{
    static auto SpawnMachineState = GetClass()->FindPropertyByName(FName(TEXT("SpawnMachineState")));

    return *SpawnMachineState->ContainerPtrToValuePtr<ESpawnMachineState>(this);
}

FScalableFloat& ABuildingGameplayActorSpawnMachine::GetResurrectionNextPlayerDelay()
{
    static auto ResurrectionNextPlayerDelay = GetClass()->FindPropertyByName(FName(TEXT("ResurrectionNextPlayerDelay")));

    return *ResurrectionNextPlayerDelay->ContainerPtrToValuePtr<FScalableFloat>(this);
}

UCapsuleComponent* ABuildingGameplayActorSpawnMachine::GetBlockingVolume() const
{
    static auto BlockingVolume = GetClass()->FindPropertyByName(FName(TEXT("BlockingVolume")));

    return *BlockingVolume->ContainerPtrToValuePtr<UCapsuleComponent*>(this);
}

UStaticMeshComponent* ABuildingGameplayActorSpawnMachine::GetPadCollision() const
{
    static auto PadCollision = GetClass()->FindPropertyByName(FName(TEXT("PadCollision")));

    return *PadCollision->ContainerPtrToValuePtr<UStaticMeshComponent*>(this);
}

FName ABuildingGameplayActorSpawnMachine::GetGrantedItemsLootTier() const
{
    static auto GrantedItemsLootTier = GetClass()->FindPropertyByName(FName(TEXT("GrantedItemsLootTier")));

    return GrantedItemsLootTier->GetPropertyValue_InContainer<FName>(this);
}

float ABuildingGameplayActorSpawnMachine::GetHeightCheckForSkydive() const
{
    static auto HeightCheckForSkydive = GetClass()->FindPropertyByName(FName(TEXT("HeightCheckForSkydive")));

    return *HeightCheckForSkydive->ContainerPtrToValuePtr<float>(this);
}

float ABuildingGameplayActorSpawnMachine::GetXYKnockAmountPlayer() const
{
    static auto XYKnockAmountPlayer = GetClass()->FindPropertyByName(FName(TEXT("XYKnockAmountPlayer")));

    return *XYKnockAmountPlayer->ContainerPtrToValuePtr<float>(this);
}

float ABuildingGameplayActorSpawnMachine::GetZKnockAmountPlayer() const
{
    static auto ZKnockAmountPlayer = GetClass()->FindPropertyByName(FName(TEXT("ZKnockAmountPlayer")));

    return *ZKnockAmountPlayer->ContainerPtrToValuePtr<float>(this);
}

float ABuildingGameplayActorSpawnMachine::GetXYKnockAmountVehicle() const
{
    static auto XYKnockAmountVehicle = GetClass()->FindPropertyByName(FName(TEXT("XYKnockAmountVehicle")));

    return *XYKnockAmountVehicle->ContainerPtrToValuePtr<float>(this);
}

float ABuildingGameplayActorSpawnMachine::GetZKnockAmountVehicle() const
{
    static auto ZKnockAmountVehicle = GetClass()->FindPropertyByName(FName(TEXT("ZKnockAmountVehicle")));

    return *ZKnockAmountVehicle->ContainerPtrToValuePtr<float>(this);
}
