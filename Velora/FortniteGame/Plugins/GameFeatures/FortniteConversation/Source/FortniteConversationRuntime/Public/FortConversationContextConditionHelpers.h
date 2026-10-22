#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ConversationContext.h"

class UFortConversationContextConditionHelpers : public UBlueprintFunctionLibrary
{
public:
    static bool GetMessageForContext(const void* Messages, const FConversationContext& Context, FText& OutText);
};
