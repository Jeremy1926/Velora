#include "pch.h"
#include "Athena/FortServerBotManagerAthena.h"
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
        .AtCall());

    auto GameMode = World ? Cast<AFortGameModeAthena>(World->GetAuthGameMode()) : nullptr;

    return GameMode ? GameMode->GetServerBotManager() : nullptr;
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
    static auto bCanStartBrain = []
    {
        auto CachedBotManager = AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("CachedBotManager"))) : nullptr;
        auto RunBehaviorTree = Scanner::Exec(TEXT("/Script/AIModule.AIController:RunBehaviorTree")).Find("call [rax+0x*]").Slot(TEXT("AAIController::RunBehaviorTree"));
        auto VTable = Scanner::FindVTable(TEXT("/Script/FortniteGame.Default__FortAthenaAIBotController"));
        auto Gate = Scanner::At(CachedBotManager && VTable && Scanner::IsSlot(RunBehaviorTree) ? VTable[RunBehaviorTree / sizeof(void*)] : nullptr).Find(std::format("mov *, [rcx+{:#x}]; test *, *; jz *; cmp byte ptr [*+0x*], 0x0", CachedBotManager ? CachedBotManager->GetOffset_ForInternal() : 0));

        return Gate.Get() ? Gate.Find("cmp byte ptr [*+0x*], 0x0").Disp(TEXT("UFortServerBotManagerAthena::CanStartBrain")) : 0;
    }();

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

    for (auto& BotInfo : GetPlayerBots())
    {
        if (auto BotController = BotInfo.GetBotController())
            BotController->RunBehaviorTree(BotController->GetBehaviorTree());
    }

    for (auto& BotInfo : GetNoneAthenaParticipantBots())
    {
        if (auto BotController = BotInfo.GetBotController())
            BotController->RunBehaviorTree(BotController->GetBehaviorTree());
    }
}

TArray<FFortServerBotInfo>& UFortServerBotManagerAthena::GetPlayerBots()
{
    static auto PlayerBots = GetClass()->FindPropertyByName(FName(TEXT("PlayerBots")));

    return *PlayerBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this);
}

TArray<FFortServerBotInfo>& UFortServerBotManagerAthena::GetNoneAthenaParticipantBots()
{
    static auto NoneAthenaParticipantBots = GetClass()->FindPropertyByName(FName(TEXT("NoneAthenaParticipantBots")));

    return *NoneAthenaParticipantBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this);
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
