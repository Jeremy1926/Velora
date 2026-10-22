#include "pch.h"
#include "Athena/AI/FortAthenaAISpawnerDataComponent_AIBotAffiliation.h"
#include "Athena/FortServerBotManagerAthena.h"
#include "AI/FortAthenaAIBotController.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

void UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned(APawn* SpawnedPawn)
{
    DETOUR(UFortAthenaAISpawnerDataComponent_AIBotAffiliation, OnSpawned, Anchor::Hook()
        .If(!Anchor::String(TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned Assigning Affiliation to Pawn=%s Controller=%s with CustomSquadId=%d")).Get())
        .String(TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::OnSpawned Can't set team Pawn=%s Controller=%s as can't retrieve AIBotController on pawn!"))
        .Optional()
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_AIBotAffiliation"))
        .LogSlot(TEXT("UFortAthenaAISpawnerDataComponent::OnSpawned"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_AIBotAffiliation")));

    Original(SpawnedPawn);

    auto BotController = SpawnedPawn ? Cast<AFortAthenaAIBotController>(SpawnedPawn->GetController()) : nullptr;
    auto ServerBotManager = BotController ? UFortServerBotManagerAthena::GetServerBotManager(SpawnedPawn->GetWorld()) : nullptr;

    if (!ServerBotManager)
        return;

    int32 SquadId = 0;

    if (GetSquadID(&SquadId))
    {
        uint8 CustomSquadId = (uint8)SquadId;

        ServerBotManager->AssignTeam(BotController, &CustomSquadId);
    }
    else
    {
        ServerBotManager->AssignTeam(BotController, nullptr);
    }
}

bool UFortAthenaAISpawnerDataComponent_AIBotAffiliation::GetSquadID(int32* OutSquadId)
{
    static auto GetSquadID_Implementation = Anchor::VirtualFunction<bool (*)(UFortAthenaAISpawnerDataComponent_AIBotAffiliation*, int32*)>(TEXT("UFortAthenaAISpawnerDataComponent_AIBotAffiliation::GetSquadID_Implementation"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAISpawnerDataComponent_AIBotAffiliation:GetSquadID"))
        .Find("call [*+0x*]")
        .Get();

    return GetSquadID_Implementation && GetSquadID_Implementation(this, OutSquadId);
}
