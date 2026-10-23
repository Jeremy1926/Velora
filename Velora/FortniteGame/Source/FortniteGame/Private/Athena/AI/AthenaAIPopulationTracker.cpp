#include "pch.h"
#include "Athena/AI/AthenaAIPopulationTracker.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UAthenaAIPopulationTracker::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaAIPopulationTracker"));

    return Class;
}

TArray<FFortServerBotInfo>* UAthenaAIPopulationTracker::GetPlayerBots()
{
    static auto PlayerBots = GetClass()->FindPropertyByName(FName(TEXT("PlayerBots")));

    return PlayerBots ? PlayerBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this) : nullptr;
}

TArray<FFortServerBotInfo>* UAthenaAIPopulationTracker::GetNoneAthenaParticipantBots()
{
    static auto NoneAthenaParticipantBots = GetClass()->FindPropertyByName(FName(TEXT("NoneAthenaParticipantBots")));
    static auto NonAthenaParticipantBots = GetClass()->FindPropertyByName(FName(TEXT("NonAthenaParticipantBots")));
    auto ParticipantBots = NoneAthenaParticipantBots ? NoneAthenaParticipantBots : NonAthenaParticipantBots;

    return ParticipantBots ? ParticipantBots->ContainerPtrToValuePtr<TArray<FFortServerBotInfo>>(this) : nullptr;
}
