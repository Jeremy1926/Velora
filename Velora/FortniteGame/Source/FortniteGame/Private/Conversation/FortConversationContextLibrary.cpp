#include "pch.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "Scanner.h"

AActor* UFortConversationContextLibrary::GetParticipantActorNPC(const FConversationContext& Context)
{
    static auto GetParticipantActorNPC = Anchor::Function<AActor* (*)(const FConversationContext&)>(TEXT("UFortConversationContextLibrary::GetParticipantActorNPC"))
        .Native(TEXT("/Script/FortniteGame.FortConversationContextLibrary:GetParticipantActorNPC"))
        .Get();

    return GetParticipantActorNPC(Context);
}

AActor* UFortConversationContextLibrary::GetParticipantActorPlayer(const FConversationContext& Context)
{
    static auto GetParticipantActorPlayer = Anchor::Function<AActor* (*)(const FConversationContext&)>(TEXT("UFortConversationContextLibrary::GetParticipantActorPlayer"))
        .Native(TEXT("/Script/FortniteGame.FortConversationContextLibrary:GetParticipantActorPlayer"))
        .Get();

    return GetParticipantActorPlayer(Context);
}

UConversationParticipantComponent* UFortConversationContextLibrary::GetParticipantComponentNPC(const FConversationContext& Context)
{
    static auto GetParticipantComponentNPC = Anchor::Function<UConversationParticipantComponent* (*)(const FConversationContext&)>(TEXT("UFortConversationContextLibrary::GetParticipantComponentNPC"))
        .Native(TEXT("/Script/FortniteGame.FortConversationContextLibrary:GetParticipantComponentNPC"))
        .Get();

    return GetParticipantComponentNPC(Context);
}

AFortPlayerController* UFortConversationContextLibrary::GetParticipantPlayerController(const FConversationContext& Context)
{
    static auto GetParticipantPlayerController = Anchor::Function<AActor* (*)(const FConversationContext&)>(TEXT("UFortConversationContextLibrary::GetParticipantPlayerController"))
        .Native(TEXT("/Script/FortniteGame.FortConversationContextLibrary:GetParticipantPlayerController"))
        .Get();

    return Cast<AFortPlayerController>(GetParticipantPlayerController(Context));
}
