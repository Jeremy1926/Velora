#include "pch.h"
#include "Athena/AI/FortAthenaAISpawnerDataComponent_AIBotConversation.h"
#include "AI/FortAthenaAIBotController.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

void UFortAthenaAISpawnerDataComponent_AIBotConversation::OnSpawned(APawn* SpawnedPawn)
{
    DETOUR(UFortAthenaAISpawnerDataComponent_AIBotConversation, OnSpawned, Anchor::Hook()
        .String(TEXT("UFortAthenaAISpawnerDataComponent_Voice::OnSpawned Pawn=%s Could not find or create a voice component. Desired component class %s, SoundBank %s"))
        .Optional()
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_Voice"))
        .LogSlot(TEXT("UFortAthenaAISpawnerDataComponent::OnSpawned"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_AIBotConversation")));

    Original(SpawnedPawn);

    auto NPCConversationParticipantComponent = SpawnedPawn && SpawnedPawn->HasAuthority() ? (UFortNPCConversationParticipantComponent*)SpawnedPawn->FindComponentByClass(UFortNPCConversationParticipantComponent::StaticClass()) : nullptr;

    if (NPCConversationParticipantComponent)
        NPCConversationParticipantComponent->InitializeServerConversation(Cast<AFortAthenaAIBotController>(SpawnedPawn->GetController()));
}
