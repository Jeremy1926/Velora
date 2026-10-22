#include "pch.h"
#include "ConversationRegistry.h"
#include "ConversationNode.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UConversationRegistry::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationRegistry"));

    return Class;
}

UConversationRegistry* UConversationRegistry::GetFromWorld(const UWorld* World)
{
    static auto WorldSubsystemClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.WorldSubsystem"));
    static auto bWorldSubsystem = WorldSubsystemClass && StaticClass()->IsChildOf(WorldSubsystemClass);

    if (bWorldSubsystem)
    {
        static auto SubsystemCollection = Anchor::Offset(TEXT("UWorld::SubsystemCollection"))
            .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ClientExecuteTaskAndSideEffects"))
            .Find("call [*+0x*]")
            .Entry(TEXT("/Script/CommonDialogueRuntime.Default__ConversationParticipantComponent"))
            .Find("call 0x*", 2)
            .Target()
            .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*")
            .Disp();
        static auto GetSubsystemInternal = Anchor::Function<UObject* (*)(void*, UClass*)>(TEXT("FSubsystemCollectionBase::GetSubsystemInternal"))
            .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ClientExecuteTaskAndSideEffects"))
            .Find("call [*+0x*]")
            .Entry(TEXT("/Script/CommonDialogueRuntime.Default__ConversationParticipantComponent"))
            .Find("call 0x*", 2)
            .Target()
            .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get();

        return World ? (UConversationRegistry*)GetSubsystemInternal((uint8*)World + SubsystemCollection, StaticClass()) : nullptr;
    }

    static auto GetFromWorld = Anchor::Function<UConversationRegistry* (*)(const UWorld*)>(TEXT("UConversationRegistry::GetFromWorld"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationParticipantComponent:ClientExecuteTaskAndSideEffects"))
        .Find("call [*+0x*]")
        .Entry(TEXT("/Script/CommonDialogueRuntime.Default__ConversationParticipantComponent"))
        .Find("call 0x*", 2)
        .Target()
        .Get();

    return World ? GetFromWorld(World) : nullptr;
}

const TMap<FGameplayTag, FConversationEntryList>& UConversationRegistry::GetEntryMap() const
{
    static auto EntryMap = StaticClass()->FindPropertyByName(FName(TEXT("EntryMap")));

    return *EntryMap->ContainerPtrToValuePtr<TMap<FGameplayTag, FConversationEntryList>>(this);
}

const TMap<FGuid, UConversationNode*>& UConversationRegistry::GetReachableNodeMap() const
{
    static auto ReachableNodeMap = StaticClass()->FindPropertyByName(FName(TEXT("ReachableNodeMap")));

    return *ReachableNodeMap->ContainerPtrToValuePtr<TMap<FGuid, UConversationNode*>>(this);
}

UConversationNode* UConversationRegistry::GetRuntimeNodeFromGUID(const FGuid& NodeGUID) const
{
    static auto ReachableNodeMap = StaticClass()->FindPropertyByName(FName(TEXT("ReachableNodeMap")));

    if (!ReachableNodeMap)
    {
        static auto NativeGetRuntimeNodeFromGUID = Anchor::Function<UConversationNode* (*)(const UConversationRegistry*, const FGuid&)>(TEXT("UConversationRegistry::GetRuntimeNodeFromGUID"))
            .String(TEXT("Start UConversationRegistry::GetRuntimeNodeFromGUID with NodeGUID: (%s)"))
            .Start()
            .Get();

        return NativeGetRuntimeNodeFromGUID(this, NodeGUID);
    }

    for (const auto& Pair : GetReachableNodeMap())
    {
        if (Pair.Key == NodeGUID)
            return Pair.Value;
    }

    return nullptr;
}

TArray<FGuid> UConversationRegistry::GetEntryPointGUIDs(FGameplayTag EntryPoint) const
{
    static auto EntryMap = StaticClass()->FindPropertyByName(FName(TEXT("EntryMap")));

    if (!EntryMap)
    {
        BuildDependenciesGraph();

        for (const auto& Pair : GetEntryTagToEntryList())
        {
            if (Pair.Key == EntryPoint)
                return Pair.Value;
        }

        return TArray<FGuid>();
    }

    for (const auto& Pair : GetEntryMap())
    {
        if (Pair.Key == EntryPoint)
            return Pair.Value.DestinationList;
    }

    return TArray<FGuid>();
}

void UConversationRegistry::BuildDependenciesGraph() const
{
    static auto BuildDependenciesGraph = Anchor::Function<void (*)(const UConversationRegistry*)>(TEXT("UConversationRegistry::BuildDependenciesGraph"))
        .String(TEXT("UConversationRegistry::BuildDependenciesGraph - Dependencies already built"))
        .Start()
        .Get();

    BuildDependenciesGraph(this);
}

const TMap<FGameplayTag, TArray<FGuid>>& UConversationRegistry::GetEntryTagToEntryList() const
{
    static auto EntryTagToEntryListAdd = Anchor::Offset(TEXT("UConversationRegistry::EntryTagToEntryList"))
        .String(TEXT("UConversationRegistry::BuildDependenciesGraph - Dependencies already built"))
        .Start()
        .Optional()
        .Find("mov rax, [rbp+0x*]; add rax, 0x*; mov [rbp-0x*], rax")
        .Find("add rax, 0x*")
        .Imm();

    static auto EntryTagToEntryList = EntryTagToEntryListAdd > 0 ? EntryTagToEntryListAdd : Anchor::Offset(TEXT("UConversationRegistry::EntryTagToEntryList"))
        .String(TEXT("UConversationRegistry::BuildDependenciesGraph - Dependencies already built"))
        .Start()
        .Find("mov rax, [rbx]; lea r12, [r13+0x*]; movzx edx, ax")
        .Find("lea r12, [r13+0x*]")
        .Disp();

    return *(const TMap<FGameplayTag, TArray<FGuid>>*)((const uint8*)this + EntryTagToEntryList);
}

TArray<FGuid> UConversationRegistry::GetOutputLinkGUIDs(FGameplayTag EntryPoint) const
{
    return GetOutputLinkGUIDs(GetEntryPointGUIDs(EntryPoint));
}

TArray<FGuid> UConversationRegistry::GetOutputLinkGUIDs(const TArray<FGuid>& SourceGUIDs) const
{
    TArray<FGuid> Result;

    for (const FGuid& SourceGUID : SourceGUIDs)
    {
        if (auto SourceNode = Cast<UConversationNodeWithLinks>(GetRuntimeNodeFromGUID(SourceGUID)))
            Result.Append(SourceNode->GetOutputConnections());
    }

    return Result;
}
