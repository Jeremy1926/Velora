#include "pch.h"
#include "Athena/AI/AthenaAIPopulationTracker.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UAthenaAIPopulationTracker::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaAIPopulationTracker"));

    return Class;
}

TArray<FFortServerBotInfo>& UAthenaAIPopulationTracker::GetPlayerBots()
{
    static auto PlayerBots = GetClass()->FindPropertyByName(FName(TEXT("PlayerBots")));

    return *PlayerBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this);
}

TArray<FFortServerBotInfo>& UAthenaAIPopulationTracker::GetNoneAthenaParticipantBots()
{
    static auto NoneAthenaParticipantBots = GetClass()->FindPropertyByName(FName(TEXT("NoneAthenaParticipantBots")));

    return *NoneAthenaParticipantBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this);
}
