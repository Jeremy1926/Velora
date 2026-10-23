#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class UFortControllerEffect;
class UScriptStruct;
struct FScalableFloat;

enum class EDataDrivenEffectRecipient : uint8
{
    Player,
    NPC
};

struct FConversationParticipantRequirement
{
    static UScriptStruct* StaticStruct();

    const FString& GetCannotUseReasonParameter() const;
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

    UObject* GetServiceBriefConfigCollection() const;

    FName GetServiceBriefCollectionKeyOveride() const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;
};
