#include "pch.h"
#include "ConversationParticipantComponent.h"
#include "ConversationInstance.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "GameFramework/Actor.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UConversationParticipantComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent"));

    return Class;
}

int32& UConversationParticipantComponent::GetConversationsActive() const
{
    static auto ConversationsActive = StaticClass()->FindPropertyByName(FName(TEXT("ConversationsActive")));

    return *ConversationsActive->ContainerPtrToValuePtr<int32>((void*)this);
}

UConversationInstance*& UConversationParticipantComponent::GetAuth_CurrentConversation() const
{
    static auto Auth_CurrentConversation = StaticClass()->FindPropertyByName(FName(TEXT("Auth_CurrentConversation")));

    return *Auth_CurrentConversation->ContainerPtrToValuePtr<UConversationInstance*>((void*)this);
}

TArray<UConversationInstance*>& UConversationParticipantComponent::GetAuth_Conversations() const
{
    static auto Auth_Conversations = StaticClass()->FindPropertyByName(FName(TEXT("Auth_Conversations")));

    return *Auth_Conversations->ContainerPtrToValuePtr<TArray<UConversationInstance*>>((void*)this);
}

void UConversationParticipantComponent::ServerNotifyConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant)
{
    AActor* Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || !Conversation)
        return;

    GetAuth_CurrentConversation() = Conversation;
    GetAuth_Conversations().Add(Conversation);

    ClientUpdateParticipants(Conversation->GetParticipantsCopy());

    if (GetConversationsActive() == 0)
        OnEnterConversationState();

    GetConversationsActive()++;

    OnServerConversationStarted(Conversation, AsParticipant);

    if (Owner->GetRemoteRole() == ROLE_AutonomousProxy)
        ClientUpdateConversations(GetConversationsActive());
}

void UConversationParticipantComponent::ServerNotifyConversationEnded(UConversationInstance* Conversation)
{
    AActor* Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || !Conversation || GetAuth_CurrentConversation() != Conversation)
        return;

    GetAuth_CurrentConversation() = nullptr;
    GetAuth_Conversations().RemoveSingle(Conversation);

    GetConversationsActive()--;

    OnServerConversationEnded(Conversation);

    if (Owner->GetRemoteRole() == ROLE_AutonomousProxy)
        ClientUpdateConversations(GetConversationsActive());

    if (GetConversationsActive() == 0)
        OnLeaveConversationState();
}

void UConversationParticipantComponent::ServerNotifyExecuteTaskAndSideEffects(const FConversationNodeHandle& Handle)
{
    AActor* Owner = GetOwner();

    if (Owner && Owner->HasAuthority())
        ClientExecuteTaskAndSideEffects(Handle);
}

void UConversationParticipantComponent::ServerAbortAllConversations()
{
    const TArray<UConversationInstance*> Conversations = GetAuth_Conversations();

    for (auto Conversation : Conversations)
    {
        if (Conversation)
            Conversation->ServerAbortConversation();
    }
}

void UConversationParticipantComponent::SendClientConversationMessage(const FConversationContext& Context, const FClientConversationMessagePayload& Payload)
{
    AActor* Owner = GetOwner();

    if (Owner && Owner->GetRemoteRole() == ROLE_AutonomousProxy)
        ClientUpdateConversation(Payload);
}

bool UConversationParticipantComponent::IsInActiveConversation() const
{
    return GetAuth_CurrentConversation() || GetConversationsActive() > 0;
}

FText UConversationParticipantComponent::GetParticipantDisplayName()
{
    static auto GetParticipantDisplayName = Anchor::VirtualFunction<FText* (*)(UConversationParticipantComponent*, FText*)>(TEXT("UConversationParticipantComponent::GetParticipantDisplayName"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:GetParticipantDisplayName"))
        .Find("call [*+0x*]")
        .Get();

    FText DisplayName(NoInit);
    GetParticipantDisplayName(this, &DisplayName);

    return DisplayName;
}

void UConversationParticipantComponent::ServerAdvanceConversation_Implementation(const FAdvanceConversationRequest& InChoicePicked)
{
    DETOUR(UConversationParticipantComponent, ServerAdvanceConversation_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ServerAdvanceConversation"))
        .Find("call [*+0x*]")
        .IsVirtual(TEXT("/Script/CommonDialogueRuntime.Default__ConversationParticipantComponent"), "ret*")
        .Optional()
        .InChildVTables(StaticClass()));

    AActor* Owner = GetOwner();

    if (Owner && Owner->HasAuthority() && GetAuth_CurrentConversation())
        GetAuth_CurrentConversation()->ServerAdvanceConversation(InChoicePicked);
}

void UConversationParticipantComponent::ClientUpdateConversation(const FClientConversationMessagePayload& Message)
{
    static auto ClientUpdateConversation = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ClientUpdateConversation"));

    static auto MetadataParametersProperty = FClientConversationMessage::StaticStruct()->FindPropertyByName(FName(TEXT("MetadataParameters")));

    FClientConversationMessagePayload Parms = Message;

    if (MetadataParametersProperty)
    {
        CallRemoteFunction(ClientUpdateConversation, &Parms, nullptr, nullptr);
        return;
    }

    alignas(FClientConversationMessagePayload) uint8 GameParms[0x68];
    memcpy(GameParms, &Parms, 0x38);
    memcpy(GameParms + 0x38, &Parms.Participants, 0x30);

    CallRemoteFunction(ClientUpdateConversation, GameParms, nullptr, nullptr);
}

void UConversationParticipantComponent::ClientUpdateConversations(int32 InConversationsActive)
{
    static auto ClientUpdateConversations = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ClientUpdateConversations"));

    int32 Parms = InConversationsActive;

    CallRemoteFunction(ClientUpdateConversations, &Parms, nullptr, nullptr);
}

void UConversationParticipantComponent::ClientUpdateParticipants(const FConversationParticipants& InParticipants)
{
    static auto ClientUpdateParticipants = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ClientUpdateParticipants"));

    FConversationParticipants Parms = InParticipants;

    CallRemoteFunction(ClientUpdateParticipants, &Parms, nullptr, nullptr);
}

void UConversationParticipantComponent::ClientExecuteTaskAndSideEffects(const FConversationNodeHandle& Handle)
{
    static auto ClientExecuteTaskAndSideEffects = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ClientExecuteTaskAndSideEffects"));

    FConversationNodeHandle Parms = Handle;

    CallRemoteFunction(ClientExecuteTaskAndSideEffects, &Parms, nullptr, nullptr);
}

void UConversationParticipantComponent::OnEnterConversationState()
{
    static auto OnEnterConversationState = Anchor::VirtualFunction<void (*)(UConversationParticipantComponent*)>(TEXT("UConversationParticipantComponent::OnEnterConversationState"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:GetParticipantDisplayName"))
        .Find("call [*+0x*]")
        .Get(sizeof(void*));

    OnEnterConversationState(this);
}

void UConversationParticipantComponent::OnLeaveConversationState()
{
    static auto OnLeaveConversationState = Anchor::VirtualFunction<void (*)(UConversationParticipantComponent*)>(TEXT("UConversationParticipantComponent::OnLeaveConversationState"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:GetParticipantDisplayName"))
        .Find("call [*+0x*]")
        .Get(sizeof(void*) * 2);

    OnLeaveConversationState(this);
}

void UConversationParticipantComponent::OnServerConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant)
{
    if (auto NPCConversationParticipantComponent = Cast<UFortNPCConversationParticipantComponent>(this))
        NPCConversationParticipantComponent->OnServerConversationStarted(Conversation, AsParticipant);
}

void UConversationParticipantComponent::OnServerConversationEnded(UConversationInstance* Conversation)
{
    if (auto NPCConversationParticipantComponent = Cast<UFortNPCConversationParticipantComponent>(this))
        NPCConversationParticipantComponent->OnServerConversationEnded(Conversation);
}
