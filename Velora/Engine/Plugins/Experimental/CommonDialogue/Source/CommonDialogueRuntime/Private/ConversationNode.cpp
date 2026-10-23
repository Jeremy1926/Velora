#include "pch.h"
#include "ConversationNode.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UConversationNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationNode"));

    return Class;
}

UObject*& UConversationNode::GetEvalWorldContextObj() const
{
    static auto EvalWorldContextObj = StaticClass()->FindPropertyByName(FName(TEXT("EvalWorldContextObj")));

    return *EvalWorldContextObj->ContainerPtrToValuePtr<UObject*>((void*)this);
}

UClass* UConversationNodeWithLinks::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CommonDialogueRuntime.ConversationNodeWithLinks"));

    return Class;
}

const TArray<FGuid>& UConversationNodeWithLinks::GetOutputConnections() const
{
    static auto OutputConnections = StaticClass()->FindPropertyByName(FName(TEXT("OutputConnections")));

    return *OutputConnections->ContainerPtrToValuePtr<TArray<FGuid>>(this);
}

FGuid UConversationNodeWithLinks::GetNodeGuid() const
{
    static auto NodeGUID = StaticClass()->FindPropertyByName(FName(TEXT("NodeGUID")));
    static auto Compiled_NodeGUID = UConversationNode::StaticClass()->FindPropertyByName(FName(TEXT("Compiled_NodeGUID")));

    return *(NodeGUID ? NodeGUID : Compiled_NodeGUID)->ContainerPtrToValuePtr<FGuid>(this);
}
