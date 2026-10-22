#include "pch.h"
#include "FortConversationContextConditionHelpers.h"
#include "Scanner.h"

bool UFortConversationContextConditionHelpers::GetMessageForContext(const void* Messages, const FConversationContext& Context, FText& OutText)
{
    static auto GetMessageForContext = Anchor::Function<bool (*)(const void*, const FConversationContext&, FText&)>(TEXT("UFortConversationContextConditionHelpers::GetMessageForContext"))
        .Native(TEXT("/Script/FortniteConversationRuntime.FortConversationContextConditionHelpers:GetMessageForContext"))
        .Get();

    return GetMessageForContext(Messages, Context, OutText);
}
