#pragma once

#include "ConversationTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

class AActor;
class UConversationInstance;
class UConversationParticipantComponent;
class UConversationRegistry;
class UConversationTaskNode;
class UWorld;
struct FFrame;

struct FConversationTaskResult
{
    static FConversationTaskResult AbortConversation();

    static FConversationTaskResult AdvanceConversation();

    static FConversationTaskResult AdvanceConversationWithChoice(const FAdvanceConversationRequest& InChoice);

    static FConversationTaskResult PauseConversationAndSendClientChoices(const FClientConversationMessage& InMessage);

    static FConversationTaskResult ReturnToLastClientChoice();

    static FConversationTaskResult ReturnToCurrentClientChoice();

    static FConversationTaskResult ReturnToConversationStart();

    bool CanConversationContinue() const
    {
        return Type != EConversationTaskResultType::AbortConversation;
    }

    EConversationTaskResultType Type = EConversationTaskResultType::Invalid;
    FAdvanceConversationRequest AdvanceToChoice;
    FClientConversationMessage Message;
};

struct FConversationContext
{
    static FConversationContext CreateServerContext(UConversationInstance* InActiveConversation, const UConversationTaskNode* InTaskBeingConsidered);

    FConversationContext CreateChildContext(const UConversationTaskNode* NewTaskBeingConsidered) const;

    FConversationContext CreateReturnScopeContext(const FConversationNodeHandle& NewReturnScope) const;

    bool IsServerContext() const
    {
        return bServer;
    }

    bool IsClientContext() const
    {
        return !bServer;
    }

    UWorld* GetWorld() const;

    FConversationNodeHandle GetCurrentNodeHandle() const;

    FConversationParticipants GetParticipantsCopy() const;

    const FConversationParticipantEntry* GetParticipant(const FGameplayTag& ParticipantTag) const;

    UConversationParticipantComponent* GetParticipantComponent(const FGameplayTag& ParticipantTag) const;

    AActor* GetParticipantActor(const FGameplayTag& ParticipantTag) const;

    UConversationRegistry* ConversationRegistry = nullptr;
    UConversationInstance* ActiveConversation = nullptr;
    UConversationParticipantComponent* ClientParticipant = nullptr;
    const UConversationTaskNode* TaskBeingConsidered = nullptr;
    TArray<FConversationNodeHandle> ReturnScopeStack;
    bool bServer = false;
};

class UConversationContextHelpers : public UBlueprintFunctionLibrary
{
public:
    static UConversationInstance* GetConversationInstance(const FConversationContext& Context);

    static void MakeConversationParticipant(const FConversationContext& Context, AActor* ParticipantActor, FGameplayTag ParticipantTag);

    static FConversationTaskResult AdvanceConversation(const FConversationContext& Context);

    static FConversationTaskResult AdvanceConversationWithChoice(const FConversationContext& Context, const FAdvanceConversationRequest& InPickedChoice);

    static FConversationTaskResult PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& Message);

    static FConversationTaskResult ReturnToLastClientChoice(const FConversationContext& Context);

    static FConversationTaskResult ReturnToCurrentClientChoice(const FConversationContext& Context);

    static FConversationTaskResult ReturnToConversationStart(const FConversationContext& Context);

    void execMakeConversationParticipant(FFrame& Stack, void* const Z_Param__Result);

    void execAdvanceConversation(FFrame& Stack, void* const Z_Param__Result);

    void execAdvanceConversationWithChoice(FFrame& Stack, void* const Z_Param__Result);

    void execPauseConversationAndSendClientChoices(FFrame& Stack, void* const Z_Param__Result);

    void execReturnToLastClientChoice(FFrame& Stack, void* const Z_Param__Result);

    void execReturnToCurrentClientChoice(FFrame& Stack, void* const Z_Param__Result);

    void execReturnToConversationStart(FFrame& Stack, void* const Z_Param__Result);
};
