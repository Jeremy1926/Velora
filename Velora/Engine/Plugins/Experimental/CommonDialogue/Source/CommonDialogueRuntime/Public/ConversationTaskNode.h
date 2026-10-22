#pragma once

#include "ConversationNode.h"
#include "ConversationContext.h"

class UConversationSubNode;

class UConversationTaskNode : public UConversationNodeWithLinks
{
public:
    static UClass* StaticClass();

    const TArray<UConversationSubNode*>& GetSubNodes() const;

    static EConversationRequirementResult MergeRequirements(EConversationRequirementResult CurrentResult, EConversationRequirementResult MergeResult);

    EConversationRequirementResult CheckRequirements(const FConversationContext& InContext) const;

    EConversationRequirementResult IsRequirementSatisfied(const FConversationContext& Context) const;

    FConversationTaskResult ExecuteTaskNode(const FConversationContext& Context) const;

    FConversationTaskResult ExecuteTaskNodeWithSideEffects(const FConversationContext& InContext) const;

    static void GenerateChoicesForDestinations(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext, const TArray<FGuid>& CandidateDestinations);

    void GatherChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& Context) const;

    void GatherStaticChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext) const;

    void GatherDynamicChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& InContext) const;

    void GatherStaticExtraData(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;
};
