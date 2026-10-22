#include "pch.h"
#include "Conversation/FortConversationChoiceWithPreviewNode.h"
#include "Conversation/FortConversationTaskNode.h"
#include "ConversationRegistry.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortConversationChoiceWithPreviewNode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationChoiceWithPreviewNode"));

    return Class;
}

bool UFortConversationChoiceWithPreviewNode::GenerateChoice(const FConversationContext& Context, FClientConversationOptionEntry& ChoiceEntry) const
{
    static auto GenerateChoice = Anchor::VirtualFunction<bool (*)(const UConversationChoiceNode*, const FConversationContext&, FClientConversationOptionEntry&)>(TEXT("UConversationChoiceNode::GenerateChoice"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationChoiceNode:FillChoice"))
        .Find("call [*+0x*]")
        .Get(sizeof(void*));

    if (!GenerateChoice(this, Context, ChoiceEntry))
        return false;

    if (!Context.TaskBeingConsidered || !Context.ConversationRegistry)
        return true;

    TArray<FGuid> SourceGUIDs;
    SourceGUIDs.Add(Context.TaskBeingConsidered->GetNodeGuid());

    for (const FGuid& DestinationGUID : Context.ConversationRegistry->GetOutputLinkGUIDs(SourceGUIDs))
    {
        auto PreviewedTaskNode = Cast<UFortConversationTaskNode>(Context.ConversationRegistry->GetRuntimeNodeFromGUID(DestinationGUID));

        if (!PreviewedTaskNode)
            continue;

        FConversationContext PreviewContext = Context.CreateChildContext(PreviewedTaskNode);
        FConversationBranchPointBuilder BranchBuilder;
        PreviewedTaskNode->GatherChoices(BranchBuilder, PreviewContext);

        FClientConversationOptionEntry PreviewedOption;
        PreviewedOption.ChoiceReference.NodeReference = DestinationGUID;

        if (BranchBuilder.Num() > 0)
            PreviewedOption = BranchBuilder.BranchPoints[0].ClientChoice;

        if (!PreviewedTaskNode->HasPreviewData(PreviewContext, PreviewedOption))
            continue;

        PreviewedTaskNode->FillPreviewData(PreviewContext, PreviewedOption, ChoiceEntry.ExtraData);
        break;
    }

    return true;
}
