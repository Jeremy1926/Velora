#include "pch.h"
#include "ConversationLibrary.h"
#include "ConversationContext.h"
#include "ConversationInstance.h"
#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UConversationInstance* UConversationLibrary::StartConversation(FGameplayTag ConversationEntryTag, AActor* Instigator, FGameplayTag InstigatorTag, AActor* Target, FGameplayTag TargetTag)
{
    DETOUR(UConversationLibrary, StartConversation, Anchor::Hook()
        .FieldPattern("48 8B 89 %o 48 89 44 24 20 E8", UFortNonPlayerConversationParticipantComponent::StaticClass() ? UFortNonPlayerConversationParticipantComponent::StaticClass()->FindPropertyByName(FName(TEXT("ConversationEntryTag"))) : nullptr)
        .Find("call 0x*")
        .IsCallTo(Scanner::Native(TEXT("/Script/CommonDialogueRuntime.ConversationLibrary:StartConversation")).Get())
        .Optional()
        .AtCall(), Anchor::Hook()
        .FieldPattern("48 8B D6 48 89 4C 24 20 48 8B 8F %o E8", UFortNonPlayerConversationParticipantComponent::StaticClass() ? UFortNonPlayerConversationParticipantComponent::StaticClass()->FindPropertyByName(FName(TEXT("ConversationEntryTag"))) : nullptr)
        .Find("call 0x*")
        .IsCallTo(Scanner::Native(TEXT("/Script/CommonDialogueRuntime.ConversationLibrary:StartConversation")).Get())
        .Optional()
        .AtCall());

    if (!Instigator || !Target || !UConversationInstance::HasServerLayout())
        return nullptr;

    UWorld* World = Instigator->GetWorld();

    if (!World)
        return nullptr;

    static auto ConversationInstanceClass = ((UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationSettings")))->FindPropertyByName(FName(TEXT("ConversationInstanceClass")));

    auto Settings = StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.Default__ConversationSettings"));
    UClass* InstanceClass = Settings ? ConversationInstanceClass->ContainerPtrToValuePtr<TSoftClassPtr<UObject>>(Settings)->LoadSynchronous() : nullptr;

    if (!InstanceClass)
        InstanceClass = UConversationInstance::StaticClass();

    auto ConversationInstance = NewObject<UConversationInstance>((UObject*)World, InstanceClass);

    if (ConversationInstance)
    {
        FConversationContext Context = FConversationContext::CreateServerContext(ConversationInstance, nullptr);

        UConversationContextHelpers::MakeConversationParticipant(Context, Target, TargetTag);
        UConversationContextHelpers::MakeConversationParticipant(Context, Instigator, InstigatorTag);

        ConversationInstance->ServerStartConversation(ConversationEntryTag);
    }

    return ConversationInstance;
}

void UConversationLibrary::execStartConversation(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UConversationLibrary, execStartConversation, Anchor::Hook()
        .If(UConversationInstance::HasServerLayout())
        .InExec(TEXT("/Script/CommonDialogueRuntime.ConversationLibrary:StartConversation")));

    FGameplayTag ConversationEntryTag;
    AActor* Instigator = nullptr;
    FGameplayTag InstigatorTag;
    AActor* Target = nullptr;
    FGameplayTag TargetTag;

    Stack.StepCompiledIn(&ConversationEntryTag);
    Stack.StepCompiledIn(&Instigator);
    Stack.StepCompiledIn(&InstigatorTag);
    Stack.StepCompiledIn(&Target);
    Stack.StepCompiledIn(&TargetTag);
    Stack.Code += !!Stack.Code;

    *(UConversationInstance**)Z_Param__Result = StartConversation(ConversationEntryTag, Instigator, InstigatorTag, Target, TargetTag);
}
