#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class UFortControllerEffect;
struct FConversationParticipantRequirement;
struct FScalableFloat;

enum class EDataDrivenEffectRecipient : uint8
{
    Player,
    NPC
};

struct FEffectRecipientConfig
{
    EDataDrivenEffectRecipient Recipient;
    TArray<UFortControllerEffect*> Effects;
};

class UFortConversationTaskNode_DataDrivenService : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    void ApplyEffects(const FConversationContext& Context) const;

    const FConversationParticipantRequirement* CheckForFailedRequirements(const FConversationContext& Context) const;

    const TArray<FEffectRecipientConfig>& GetEffectRecipientConfigs() const;

    FName GetPricingKey() const;

    int32 GetPricingColumn() const;

    const FScalableFloat& GetLimitedStockCount() const;
};
