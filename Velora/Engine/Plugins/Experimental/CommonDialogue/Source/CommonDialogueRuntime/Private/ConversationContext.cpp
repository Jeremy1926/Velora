#include "pch.h"
#include "ConversationContext.h"
#include "ConversationInstance.h"
#include "ConversationParticipantComponent.h"
#include "ConversationRegistry.h"
#include "GameFramework/Actor.h"
#include "UObject/Stack.h"
#include "Scanner.h"
#include "Detour.h"

FConversationTaskResult FConversationTaskResult::AbortConversation()
{
    FConversationTaskResult Result;
    Result.Type = EConversationTaskResultType::AbortConversation;

    return Result;
}

FConversationTaskResult FConversationTaskResult::AdvanceConversation()
{
    FConversationTaskResult Result;
    Result.Type = EConversationTaskResultType::AdvanceConversation;

    return Result;
}

FConversationTaskResult FConversationTaskResult::AdvanceConversationWithChoice(const FAdvanceConversationRequest& InChoice)
{
    FConversationTaskResult Result;
    Result.Type = EConversationTaskResultType::AdvanceConversationWithChoice;
    Result.AdvanceToChoice = InChoice;

    return Result;
}

FConversationTaskResult FConversationTaskResult::PauseConversationAndSendClientChoices(const FClientConversationMessage& InMessage)
{
    FConversationTaskResult Result;
    Result.Type = EConversationTaskResultType::PauseConversationAndSendClientChoices;
    Result.Message = InMessage;

    return Result;
}

FConversationTaskResult FConversationTaskResult::ReturnToLastClientChoice()
{
    FConversationTaskResult Result;
    Result.Type = EConversationTaskResultType::ReturnToLastClientChoice;

    return Result;
}

FConversationTaskResult FConversationTaskResult::ReturnToCurrentClientChoice()
{
    FConversationTaskResult Result;
    Result.Type = EConversationTaskResultType::ReturnToCurrentClientChoice;

    return Result;
}

FConversationTaskResult FConversationTaskResult::ReturnToConversationStart()
{
    FConversationTaskResult Result;
    Result.Type = EConversationTaskResultType::ReturnToConversationStart;

    return Result;
}

FConversationContext FConversationContext::CreateServerContext(UConversationInstance* InActiveConversation, const UConversationTaskNode* InTaskBeingConsidered)
{
    FConversationContext Context;
    Context.ActiveConversation = InActiveConversation;
    Context.TaskBeingConsidered = InTaskBeingConsidered;
    Context.bServer = true;
    Context.ConversationRegistry = UConversationRegistry::GetFromWorld(InActiveConversation->GetWorld());

    return Context;
}

FConversationContext FConversationContext::CreateChildContext(const UConversationTaskNode* NewTaskBeingConsidered) const
{
    FConversationContext Context = *this;
    Context.TaskBeingConsidered = NewTaskBeingConsidered;

    return Context;
}

FConversationContext FConversationContext::CreateReturnScopeContext(const FConversationNodeHandle& NewReturnScope) const
{
    FConversationContext Context = *this;
    Context.ReturnScopeStack.Add(NewReturnScope);

    return Context;
}

UWorld* FConversationContext::GetWorld() const
{
    if (bServer)
        return ActiveConversation ? ActiveConversation->GetWorld() : nullptr;

    return ClientParticipant ? ClientParticipant->GetWorld() : nullptr;
}

FConversationNodeHandle FConversationContext::GetCurrentNodeHandle() const
{
    if (bServer && ActiveConversation)
        return ActiveConversation->GetCurrentNodeHandle();

    return FConversationNodeHandle();
}

FConversationParticipants FConversationContext::GetParticipantsCopy() const
{
    if (bServer && ActiveConversation)
        return ActiveConversation->GetParticipantsCopy();

    return FConversationParticipants();
}

const FConversationParticipantEntry* FConversationContext::GetParticipant(const FGameplayTag& ParticipantTag) const
{
    if (bServer && ActiveConversation)
        return ActiveConversation->GetParticipant(ParticipantTag);

    return nullptr;
}

UConversationParticipantComponent* FConversationContext::GetParticipantComponent(const FGameplayTag& ParticipantTag) const
{
    const FConversationParticipantEntry* Entry = GetParticipant(ParticipantTag);

    return Entry ? Entry->GetParticipantComponent() : nullptr;
}

AActor* FConversationContext::GetParticipantActor(const FGameplayTag& ParticipantTag) const
{
    const FConversationParticipantEntry* Entry = GetParticipant(ParticipantTag);

    return Entry ? Entry->Actor : nullptr;
}

UConversationInstance* UConversationContextHelpers::GetConversationInstance(const FConversationContext& Context)
{
    return Context.ActiveConversation;
}

void UConversationContextHelpers::MakeConversationParticipant(const FConversationContext& Context, AActor* ParticipantActor, FGameplayTag ParticipantTag)
{
    if (!ParticipantActor || !ParticipantTag.IsValid())
        return;

    if (UConversationInstance* Conversation = GetConversationInstance(Context))
        Conversation->ServerAssignParticipant(ParticipantTag, ParticipantActor);
}

FConversationTaskResult UConversationContextHelpers::AdvanceConversation(const FConversationContext& Context)
{
    return FConversationTaskResult::AdvanceConversation();
}

FConversationTaskResult UConversationContextHelpers::AdvanceConversationWithChoice(const FConversationContext& Context, const FAdvanceConversationRequest& InPickedChoice)
{
    return FConversationTaskResult::AdvanceConversationWithChoice(InPickedChoice);
}

FConversationTaskResult UConversationContextHelpers::PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& Message)
{
    return FConversationTaskResult::PauseConversationAndSendClientChoices(Message);
}

FConversationTaskResult UConversationContextHelpers::ReturnToLastClientChoice(const FConversationContext& Context)
{
    return FConversationTaskResult::ReturnToLastClientChoice();
}

FConversationTaskResult UConversationContextHelpers::ReturnToCurrentClientChoice(const FConversationContext& Context)
{
    return FConversationTaskResult::ReturnToCurrentClientChoice();
}

FConversationTaskResult UConversationContextHelpers::ReturnToConversationStart(const FConversationContext& Context)
{
    return FConversationTaskResult::ReturnToConversationStart();
}

void UConversationContextHelpers::execMakeConversationParticipant(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationContextHelpers, execMakeConversationParticipant, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .Native(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:MakeConversationParticipant"))
        .Is("ret*")
        .Optional()
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:MakeConversationParticipant")));

    FConversationContext ContextTemp;
    AActor* ParticipantActor = nullptr;
    FGameplayTag ParticipantTag;

    const FConversationContext& Context = Stack.StepCompiledInRef<FConversationContext>(&ContextTemp);
    Stack.StepCompiledIn(&ParticipantActor);
    Stack.StepCompiledIn(&ParticipantTag);
    Stack.Code += !!Stack.Code;

    MakeConversationParticipant(Context, ParticipantActor, ParticipantTag);
}

void UConversationContextHelpers::execAdvanceConversation(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationContextHelpers, execAdvanceConversation, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:AdvanceConversation")));

    FConversationContext ContextTemp;

    const FConversationContext& Context = Stack.StepCompiledInRef<FConversationContext>(&ContextTemp);
    Stack.Code += !!Stack.Code;

    *(FConversationTaskResult*)Z_Param__Result = AdvanceConversation(Context);
}

void UConversationContextHelpers::execAdvanceConversationWithChoice(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationContextHelpers, execAdvanceConversationWithChoice, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:AdvanceConversationWithChoice")));

    FConversationContext ContextTemp;
    FAdvanceConversationRequest ChoiceTemp;

    const FConversationContext& Context = Stack.StepCompiledInRef<FConversationContext>(&ContextTemp);
    const FAdvanceConversationRequest& Choice = Stack.StepCompiledInRef<FAdvanceConversationRequest>(&ChoiceTemp);
    Stack.Code += !!Stack.Code;

    *(FConversationTaskResult*)Z_Param__Result = AdvanceConversationWithChoice(Context, Choice);
}

void UConversationContextHelpers::execPauseConversationAndSendClientChoices(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationContextHelpers, execPauseConversationAndSendClientChoices, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:PauseConversationAndSendClientChoices")));

    FConversationContext ContextTemp;
    FClientConversationMessage MessageTemp;

    const FConversationContext& Context = Stack.StepCompiledInRef<FConversationContext>(&ContextTemp);
    const FClientConversationMessage& Message = Stack.StepCompiledInRef<FClientConversationMessage>(&MessageTemp);
    Stack.Code += !!Stack.Code;

    *(FConversationTaskResult*)Z_Param__Result = PauseConversationAndSendClientChoices(Context, Message);
}

void UConversationContextHelpers::execReturnToLastClientChoice(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationContextHelpers, execReturnToLastClientChoice, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:ReturnToLastClientChoice")));

    FConversationContext ContextTemp;

    const FConversationContext& Context = Stack.StepCompiledInRef<FConversationContext>(&ContextTemp);
    Stack.Code += !!Stack.Code;

    *(FConversationTaskResult*)Z_Param__Result = ReturnToLastClientChoice(Context);
}

void UConversationContextHelpers::execReturnToCurrentClientChoice(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationContextHelpers, execReturnToCurrentClientChoice, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:ReturnToCurrentClientChoice")));

    FConversationContext ContextTemp;

    const FConversationContext& Context = Stack.StepCompiledInRef<FConversationContext>(&ContextTemp);
    Stack.Code += !!Stack.Code;

    *(FConversationTaskResult*)Z_Param__Result = ReturnToCurrentClientChoice(Context);
}

void UConversationContextHelpers::execReturnToConversationStart(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationContextHelpers, execReturnToConversationStart, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationContextHelpers:ReturnToConversationStart")));

    FConversationContext ContextTemp;

    const FConversationContext& Context = Stack.StepCompiledInRef<FConversationContext>(&ContextTemp);
    Stack.Code += !!Stack.Code;

    *(FConversationTaskResult*)Z_Param__Result = ReturnToConversationStart(Context);
}
