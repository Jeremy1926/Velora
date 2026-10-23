#include "pch.h"
#include "FortAthenaLivingWorldEncounter.h"
#include "FortAthenaLivingWorldManager.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FFortAthenaLivingWorldEncounterStage::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEncounterStage"));

    return Struct;
}

int32 FFortAthenaLivingWorldEncounterStage::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

TSoftObjectPtr<UDataTable>& FFortAthenaLivingWorldEncounterStage::GetEventTable()
{
    static auto EventTable = StaticStruct()->FindPropertyByName(FName(TEXT("EventTable")));

    return *EventTable->ContainerPtrToValuePtr<TSoftObjectPtr<UDataTable>>(this);
}

FScalableFloat& FFortAthenaLivingWorldEncounterStage::GetMaximumConcurrentNumberOfAI()
{
    static auto MaximumConcurrentNumberOfAI = StaticStruct()->FindPropertyByName(FName(TEXT("MaximumConcurrentNumberOfAI")));

    return *MaximumConcurrentNumberOfAI->ContainerPtrToValuePtr<FScalableFloat>(this);
}

UClass* UFortAthenaLivingWorldEncounter::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEncounter"));

    return Class;
}

FScalableFloat& UFortAthenaLivingWorldEncounter::GetIntervalBetweenEventGeneration()
{
    static auto IntervalBetweenEventGeneration = GetClass()->FindPropertyByName(FName(TEXT("IntervalBetweenEventGeneration")));

    return *IntervalBetweenEventGeneration->ContainerPtrToValuePtr<FScalableFloat>(this);
}

TRuntimeSizedArray<FFortAthenaLivingWorldEncounterStage>& UFortAthenaLivingWorldEncounter::GetStages()
{
    static auto Stages = GetClass()->FindPropertyByName(FName(TEXT("Stages")));

    return *Stages->ContainerPtrToValuePtr<TRuntimeSizedArray<FFortAthenaLivingWorldEncounterStage>>(this);
}

UClass* UFortAthenaLivingWorldEncounterInstance::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEncounterInstance"));

    return Class;
}

int32 UFortAthenaLivingWorldEncounterInstance::GetTotalActorCount()
{
    auto World = GetWorld();
    auto GameState = World ? World->GetGameState() : nullptr;
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass()) : nullptr;

    return LivingWorldManager ? LivingWorldManager->GetTotalEncounterInstanceActorCount(this) : 0;
}

void UFortAthenaLivingWorldEncounterInstance::execGetTotalActorCount(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortAthenaLivingWorldEncounterInstance, execGetTotalActorCount, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetEncounterInstanceOffset() > 0)
        .StubExec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEncounterInstance:GetTotalActorCount"))
        .Optional()
        .InExec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEncounterInstance:GetTotalActorCount")));

    Stack.Code += !!Stack.Code;

    *(int32*)Z_Param__Result = GetTotalActorCount();
}

void UFortAthenaLivingWorldEncounterInstance::TryAddActivePointProvider(const FScriptInterface& PointProvider)
{
    auto& RegisteredPointProviders = GetRegisteredPointProviders();

    for (auto& OriginalPointProvider : GetOriginalPointProviders())
    {
        if (OriginalPointProvider.GetObject() != PointProvider.GetObject())
            continue;

        for (auto& RegisteredPointProvider : RegisteredPointProviders)
        {
            if (RegisteredPointProvider.GetObject() == PointProvider.GetObject())
                return;
        }

        RegisteredPointProviders.Add(PointProvider);
        return;
    }
}

void UFortAthenaLivingWorldEncounterInstance::RemoveActivePointProvider(const FScriptInterface& PointProvider)
{
    auto& RegisteredPointProviders = GetRegisteredPointProviders();

    for (int32 Index = 0; Index < RegisteredPointProviders.Num(); Index++)
    {
        if (RegisteredPointProviders[Index].GetObject() == PointProvider.GetObject())
        {
            RegisteredPointProviders.RemoveAt(Index);
            return;
        }
    }
}

TSoftObjectPtr<UFortAthenaLivingWorldEncounter>& UFortAthenaLivingWorldEncounterInstance::GetEncounterDefinition()
{
    static auto EncounterDefinition = GetClass()->FindPropertyByName(FName(TEXT("EncounterDefinition")));

    return *EncounterDefinition->ContainerPtrToValuePtr<TSoftObjectPtr<UFortAthenaLivingWorldEncounter>>(this);
}

TArray<FScriptInterface>& UFortAthenaLivingWorldEncounterInstance::GetOriginalPointProviders()
{
    static auto OriginalPointProviders = GetClass()->FindPropertyByName(FName(TEXT("OriginalPointProviders")));

    return *OriginalPointProviders->ContainerPtrToValuePtr<TArray<FScriptInterface>>(this);
}

TArray<FScriptInterface>& UFortAthenaLivingWorldEncounterInstance::GetRegisteredPointProviders()
{
    static auto RegisteredPointProviders = GetClass()->FindPropertyByName(FName(TEXT("RegisteredPointProviders")));

    return *RegisteredPointProviders->ContainerPtrToValuePtr<TArray<FScriptInterface>>(this);
}

AActor* UFortAthenaLivingWorldEncounterInstance::GetActorDensityReservoir()
{
    static auto ActorDensityReservoir = GetClass()->FindPropertyByName(FName(TEXT("ActorDensityReservoir")));

    return *ActorDensityReservoir->ContainerPtrToValuePtr<AActor*>(this);
}

FFortAthenaLivingWorldCategory*& UFortAthenaLivingWorldEncounterInstance::GetCategory()
{
    static auto Category = Anchor::Offset(TEXT("UFortAthenaLivingWorldEncounterInstance::Category"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov rdx, rax; mov r8, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rdx, *; mov [rsp+0x20], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov [rcx+0x*], rdx")
        .Disp();

    return *(FFortAthenaLivingWorldCategory**)((uint8*)this + Category);
}

int32& UFortAthenaLivingWorldEncounterInstance::GetCurrentStageIndex()
{
    static auto CurrentStageIndex = GetClass()->FindPropertyByName(FName(TEXT("CurrentStageIndex")));

    return *CurrentStageIndex->ContainerPtrToValuePtr<int32>(this);
}

float& UFortAthenaLivingWorldEncounterInstance::GetLastGenerationTime()
{
    static auto LastGenerationTime = Anchor::Offset(TEXT("UFortAthenaLivingWorldEncounterInstance::LastGenerationTime"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldEncounterInstance:RequestEventGeneration"))
        .Find("and dword ptr [rbx+0x*], 0x0")
        .Disp();

    return *(float*)((uint8*)this + LastGenerationTime);
}

bool UFortAthenaLivingWorldEncounterInstance::GetbIsPaused()
{
    static auto bIsPaused = GetClass()->FindPropertyByName(FName(TEXT("bIsPaused")));

    return *bIsPaused->ContainerPtrToValuePtr<bool>(this);
}

FMulticastScriptDelegate& UFortAthenaLivingWorldEncounterInstance::GetOnActorSpawned()
{
    static auto OnActorSpawned = GetClass()->FindPropertyByName(FName(TEXT("OnActorSpawned")));

    return *OnActorSpawned->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

void* UFortAthenaLivingWorldEncounterInstance::GetEventRuntimeDataMap()
{
    static auto EventRuntimeDataMap = Anchor::Offset(TEXT("UFortAthenaLivingWorldEncounterInstance::EventRuntimeDataMap"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:StartEncounter"))
        .Find("mov rdx, rax; mov r8, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rdx, [*+0x*]; mov rcx, *; call 0x*; mov r8, [rsp+0x*]; lea r9, [*+0x*]")
        .Disp();

    return (uint8*)this + EventRuntimeDataMap;
}
