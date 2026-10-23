#include "pch.h"
#include "Athena/FortServerBotManagerAthena.h"
#include "Athena/AI/AthenaAIPopulationTracker.h"
#include "AI/FortAthenaAIBotController.h"
#include "Athena/FortGameModeAthena.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

struct FPendingBotBrainActivation
{
    TWeakObjectPtr<UFortServerBotManagerAthena> ServerBotManager;
    float Time;
};

static TArray<FPendingBotBrainActivation> PendingBotBrainActivations;

const UFortAthenaAIBotCustomizationData* UFortServerBotManagerAthena::SpawningBotData = nullptr;

const FFortAthenaAIBotRunTimeCustomizationData* UFortServerBotManagerAthena::SpawningRuntimeBotData = nullptr;

UScriptStruct* FFortServerBotInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortServerBotInfo"));

    return Struct;
}

int32 FFortServerBotInfo::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

AFortAthenaAIBotController*& FFortServerBotInfo::GetBotController()
{
    static auto BotController = StaticStruct()->FindPropertyByName(FName(TEXT("BotController")));

    return *BotController->ContainerPtrToValuePtr<AFortAthenaAIBotController*>(this);
}

UClass* UFortServerBotManagerAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortServerBotManagerAthena"));

    return Class;
}

UFortServerBotManagerAthena* UFortServerBotManagerAthena::GetServerBotManager(UWorld* World)
{
    DETOUR(UFortServerBotManagerAthena, GetServerBotManager, Anchor::Hook()
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("call 0x*", 2)
        .Target()
        .Find("ret")
        .Last("call 0x*")
        .Target()
        .Is("push *; sub rsp, 0x20; mov rax, [rcx]; mov *, rcx; call [rax+0x*]; mov rcx, rax; call 0x*")
        .Optional()
        .Find("call 0x*")
        .AtCall(), Anchor::Hook()
        .String(TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned Can't set team Pawn=%s Controller=%s as can't retrieve AIBotController on pawn!"))
        .Start()
        .Find("or [*+0x*], al; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .IsTarget("xor eax, eax; ret*")
        .Optional()
        .AtCall());

    auto GameMode = World ? Cast<AFortGameModeAthena>(World->GetAuthGameMode()) : nullptr;

    return GameMode ? GameMode->GetServerBotManager() : nullptr;
}

AFortPlayerPawnAthena* UFortServerBotManagerAthena::SpawnBot(const FVector& InSpawnLocation, const FRotator& InSpawnRotation, const UFortAthenaAIBotCustomizationData* InBotData, const FFortAthenaAIBotRunTimeCustomizationData& InRuntimeBotData)
{
    DETOUR(UFortServerBotManagerAthena, SpawnBot, Anchor::Hook()
        .If(!::Anchor::Exists()
            .String(TEXT("UFortServerBotManagerAthena::SpawnBot error occured during the actor spawning"))
            .Start()
            .Find("test *, 0x*; jz *; mov r9, *; lea r8, [*]; mov rdx, *; mov rcx, *; call 0x*")
            .Get())
        .String(TEXT("UFortServerBotManagerAthena::SpawnBot error occured during the actor spawning"))
        .Start()
        .Has("test byte ptr [*+0x3b], 0x8; cmovz *, *")
        .Optional()
        .Inline());

    SpawningBotData = InBotData;
    SpawningRuntimeBotData = &InRuntimeBotData;

    auto Pawn = Original(InSpawnLocation, InSpawnRotation, InBotData, InRuntimeBotData);

    SpawningBotData = nullptr;
    SpawningRuntimeBotData = nullptr;

    return Pawn;
}

void UFortServerBotManagerAthena::AssignTeam(AFortAthenaAIBotController* BotController, const uint8* CustomSquadId)
{
    static auto AssignTeam = Anchor::Function<void (*)(UFortServerBotManagerAthena*, AFortAthenaAIBotController*, const uint8*)>(TEXT("UFortServerBotManagerAthena::AssignTeam"))
        .String(TEXT("No more team for this Bot:%s"))
        .Start()
        .Get();

    if (AssignTeam)
        AssignTeam(this, BotController, CustomSquadId);
}

void UFortServerBotManagerAthena::SetBotBrainActivationTimer()
{
    PendingBotBrainActivations.Add(FPendingBotBrainActivation{ this, GetWorld()->GetTimeSeconds() + 1.0f });
}

void UFortServerBotManagerAthena::TickBotBrainActivationTimers()
{
    for (int32 Index = PendingBotBrainActivations.Num() - 1; Index >= 0; Index--)
    {
        auto ServerBotManager = PendingBotBrainActivations[Index].ServerBotManager.Get();

        if (!ServerBotManager)
        {
            PendingBotBrainActivations.RemoveAt(Index);
            continue;
        }

        if (ServerBotManager->GetWorld()->GetTimeSeconds() < PendingBotBrainActivations[Index].Time)
            continue;

        PendingBotBrainActivations[Index].Time += 1.0f;

        ServerBotManager->CheckForBotBrainActivation();
    }
}

void UFortServerBotManagerAthena::CheckForBotBrainActivation()
{
    static auto bCanStartBrain = Anchor::Offset(TEXT("UFortServerBotManagerAthena::CanStartBrain"))
        .Exec(TEXT("/Script/AIModule.AIController:RunBehaviorTree"))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("AAIController::RunBehaviorTree"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortAthenaAIBotController"))
        .FieldFind("mov *, [rcx+%x]; test *, *; jz *; cmp *[*+0x*], *", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("CachedBotManager"))) : nullptr)
        .Optional()
        .Find("cmp *[*+0x*], *")
        .Disp();

    auto World = GetWorld();

    if (!World)
        return;

    if (bCanStartBrain)
    {
        for (auto StreamingLevel : World->GetStreamingLevels())
        {
            if (StreamingLevel->GetCurrentState() == ULevelStreaming::ECurrentState::FailedToLoad)
                continue;

            if (!StreamingLevel->IsLevelVisible() && StreamingLevel->ShouldBeVisible())
                return;

            if (StreamingLevel->IsLevelLoaded() && !StreamingLevel->ShouldBeLoaded())
                return;
        }
    }

    for (int32 Index = PendingBotBrainActivations.Num() - 1; Index >= 0; Index--)
    {
        if (PendingBotBrainActivations[Index].ServerBotManager.Get() == this)
            PendingBotBrainActivations.RemoveAt(Index);
    }

    if (!bCanStartBrain)
        return;

    *((uint8*)this + bCanStartBrain) = true;

    for (auto BotInfos : { GetPlayerBots(), GetNoneAthenaParticipantBots() })
    {
        if (!BotInfos)
            continue;

        for (auto& BotInfo : *BotInfos)
        {
            if (auto BotController = BotInfo.GetBotController())
                BotController->RunBehaviorTree(BotController->GetBehaviorTree());
        }
    }
}

TArray<FFortServerBotInfo>* UFortServerBotManagerAthena::GetPlayerBots()
{
    static auto PlayerBots = GetClass()->FindPropertyByName(FName(TEXT("PlayerBots")));

    if (PlayerBots)
        return PlayerBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this);

    auto AIPopulationTracker = GetCachedAIPopulationTracker();

    return AIPopulationTracker ? &AIPopulationTracker->GetPlayerBots() : nullptr;
}

TArray<FFortServerBotInfo>* UFortServerBotManagerAthena::GetNoneAthenaParticipantBots()
{
    static auto NoneAthenaParticipantBots = GetClass()->FindPropertyByName(FName(TEXT("NoneAthenaParticipantBots")));

    if (NoneAthenaParticipantBots)
        return NoneAthenaParticipantBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this);

    auto AIPopulationTracker = GetCachedAIPopulationTracker();

    return AIPopulationTracker ? &AIPopulationTracker->GetNoneAthenaParticipantBots() : nullptr;
}

AFortGameModeAthena*& UFortServerBotManagerAthena::GetCachedGameMode()
{
    static auto CachedGameMode = GetClass()->FindPropertyByName(FName(TEXT("CachedGameMode")));

    return *CachedGameMode->ContainerPtrToValuePtr<AFortGameModeAthena*>(this);
}

AFortGameStateAthena*& UFortServerBotManagerAthena::GetCachedGameState()
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    return *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>(this);
}

UAthenaAIPopulationTracker* UFortServerBotManagerAthena::GetCachedAIPopulationTracker()
{
    static auto CachedAIPopulationTracker = GetClass()->FindPropertyByName(FName(TEXT("CachedAIPopulationTracker")));

    return CachedAIPopulationTracker ? *CachedAIPopulationTracker->ContainerPtrToValuePtr<UAthenaAIPopulationTracker*>(this) : nullptr;
}

void UFortServerBotManagerAthena::SetCachedAIPopulationTracker(UAthenaAIPopulationTracker* InCachedAIPopulationTracker)
{
    static auto CachedAIPopulationTracker = GetClass()->FindPropertyByName(FName(TEXT("CachedAIPopulationTracker")));

    if (CachedAIPopulationTracker)
        *CachedAIPopulationTracker->ContainerPtrToValuePtr<UAthenaAIPopulationTracker*>(this) = InCachedAIPopulationTracker;
}

FSimpleMulticastDelegate* UFortServerBotManagerAthena::GetOnNoAIAlive()
{
    static auto OnNoAIAlive = Anchor::Offset(TEXT("UFortServerBotManagerAthena::OnNoAIAlive"))
        .String(TEXT("Can't update bot count if no GameMode set..."))
        .Find("add eax, [rcx+0x*]")
        .Find("lea rcx, [*+0x*]; call 0x*")
        .Disp();

    return OnNoAIAlive > 0 ? (FSimpleMulticastDelegate*)((uint8*)this + OnNoAIAlive) : nullptr;
}

TMulticastDelegate<void, AFortAthenaAIBotController*>* UFortServerBotManagerAthena::GetOnBotController()
{
    auto OnNoAIAlive = GetOnNoAIAlive();

    return OnNoAIAlive ? (TMulticastDelegate<void, AFortAthenaAIBotController*>*)(OnNoAIAlive + 1) : nullptr;
}
