#include "pch.h"
#include "Athena/AI/AthenaAISystem.h"
#include "Athena/FortServerBotManagerAthena.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UAthenaAISystem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaAISystem"));

    return Class;
}

UAthenaAIPerceptionManager* UAthenaAISystem::GetPerceptionManager() const
{
    static auto PerceptionManager = GetClass()->FindPropertyByName(FName(TEXT("PerceptionManager")));

    return *PerceptionManager->ContainerPtrToValuePtr<UAthenaAIPerceptionManager*>(this);
}

UAthenaAIPopulationTracker* UAthenaAISystem::GetAIPopulationTracker() const
{
    static auto AIPopulationTracker = GetClass()->FindPropertyByName(FName(TEXT("AIPopulationTracker")));

    return AIPopulationTracker ? *AIPopulationTracker->ContainerPtrToValuePtr<UAthenaAIPopulationTracker*>(this) : nullptr;
}

UAthenaAISpawner* UAthenaAISystem::GetAISpawner() const
{
    static auto AISpawner = GetClass()->FindPropertyByName(FName(TEXT("AISpawner")));

    return *AISpawner->ContainerPtrToValuePtr<UAthenaAISpawner*>(this);
}

UFortServerBotManagerAthena*& UAthenaAISystem::GetPlayerBotManager()
{
    static auto PlayerBotManager = GetClass()->FindPropertyByName(FName(TEXT("PlayerBotManager")));

    return *PlayerBotManager->ContainerPtrToValuePtr<UFortServerBotManagerAthena*>(this);
}

void UAthenaAISystem::OnNavigationReady()
{
    DETOUR(UAthenaAISystem, OnNavigationReady, Anchor::Hook()
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("call 0x*", 2)
        .Target()
        .Find("ret")
        .Last("call 0x*")
        .Target()
        .Is("push rbx; sub rsp, 0x20; mov rax, [rcx]; mov rbx, rcx; call [rax+0x*]; mov rcx, rbx; add rsp, 0x20; pop rbx; jmp 0x*")
        .Optional()
        .Inline(), Anchor::Hook()
        .If(Anchor::Address()
            .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
            .Find("call 0x*")
            .Target()
            .Find("ret")
            .Last("call 0x*")
            .Target()
            .Is("mov rax, rsp; mov [rax+0x8], rbx; mov [rax+0x10], rbp; mov [rax+0x18], rsi; mov [rax+0x20], rdi; push r14; sub rsp, 0x20; mov rax, [rcx]; mov rdi, rcx; call [rax+0x*]")
            .Get() != nullptr)
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("call 0x*")
        .Target()
        .Find("ret")
        .Last("call 0x*")
        .Optional()
        .AtCall());

    auto& PlayerBotManager = GetPlayerBotManager();

    PlayerBotManager = UFortServerBotManagerAthena::GetServerBotManager(GetWorld());

    if (PlayerBotManager)
    {
        if (auto OnNoAIAlive = PlayerBotManager->GetOnNoAIAlive())
            OnNoAIAlive->AddUObject(this, &UAthenaAISystem::UpdateNavigationLocks);

        if (auto OnBotController = PlayerBotManager->GetOnBotController())
            OnBotController->AddUObject(this, &UAthenaAISystem::HandleBotControllerEvent);
    }

    Original();
}

void UAthenaAISystem::UpdateNavigationLocks()
{
    static auto UpdateNavigationLocks = Anchor::Function<void (*)(UAthenaAISystem*)>(TEXT("UAthenaAISystem::UpdateNavigationLocks"))
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("call 0x*", 2)
        .Target()
        .Find("ret")
        .Last("call 0x*")
        .Target()
        .Is("push rbx; sub rsp, 0x20; mov rax, [rcx]; mov rbx, rcx; call [rax+0x*]; mov rcx, rbx; add rsp, 0x20; pop rbx; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("call 0x*")
        .Target()
        .Find("ret")
        .Last("call 0x*")
        .Target()
        .Is("mov rax, rsp; mov [rax+0x8], rbx; mov [rax+0x10], rbp; mov [rax+0x18], rsi; mov [rax+0x20], rdi; push r14; sub rsp, 0x20; mov rax, [rcx]; mov rdi, rcx; call [rax+0x*]")
        .Get();

    if (UpdateNavigationLocks)
        UpdateNavigationLocks(this);
}

void UAthenaAISystem::HandleBotControllerEvent(AFortAthenaAIBotController* BotController)
{
    UpdateNavigationLocks();
}
