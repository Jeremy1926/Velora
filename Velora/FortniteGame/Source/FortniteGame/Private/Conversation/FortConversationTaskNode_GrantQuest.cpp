#include "pch.h"
#include "Conversation/FortConversationTaskNode_GrantQuest.h"
#include "Quests/FortQuestItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortConversationTaskNode_GrantQuest::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_GrantQuest"));

    return Class;
}

bool UFortConversationTaskNode_GrantQuest::GetbShouldDisplayQuestProviderImage() const
{
    static auto bShouldDisplayQuestProviderImage = StaticClass()->FindPropertyByName(FName(TEXT("bShouldDisplayQuestProviderImage")));

    return *bShouldDisplayQuestProviderImage->ContainerPtrToValuePtr<bool>(this);
}

TSoftObjectPtr<UFortQuestItemDefinition> UFortConversationTaskNode_GrantQuest::GetQuestToGrant(const FConversationContext& Context) const
{
    static auto GetQuestToGrant = Anchor::VirtualFunction<TSoftObjectPtr<UFortQuestItemDefinition>* (*)(const UFortConversationTaskNode_GrantQuest*, TSoftObjectPtr<UFortQuestItemDefinition>*, const FConversationContext&)>(TEXT("UFortConversationTaskNode_GrantQuest::GetQuestToGrant"))
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode:GetTaskIcon"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_GrantQuest"), -0x18)
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortConversationTaskNode:GetTaskIcon"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_GrantQuest"), 0x28)
        .Get();

    alignas(TSoftObjectPtr<UFortQuestItemDefinition>) uint8 QuestToGrantStorage[sizeof(TSoftObjectPtr<UFortQuestItemDefinition>)];
    auto QuestToGrant = GetQuestToGrant(this, (TSoftObjectPtr<UFortQuestItemDefinition>*)QuestToGrantStorage, Context);
    TSoftObjectPtr<UFortQuestItemDefinition> Result = *QuestToGrant;

    QuestToGrant->~TSoftObjectPtr();

    return Result;
}

void UFortConversationTaskNode_GrantQuest::GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    UFortConversationTaskNode::GatherStaticExtraData_Implementation(Context, InOutExtraData);

    const FString QuestToGrant = GetQuestToGrant(Context).ToString();

    if (QuestToGrant.IsEmpty())
        return;

    InOutExtraData.Add({ FString(TEXT("quest")), QuestToGrant });
    InOutExtraData.Add({ FString(TEXT("displayimage")), FString(GetbShouldDisplayQuestProviderImage() ? TEXT("1") : TEXT("0")) });
}
