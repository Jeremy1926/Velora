#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"

class AFortPlayerPawn;

enum class EPreventAbilityUseReason : uint8
{
    CannotAfford,
    AlreadyActive,
    AbilityActivationBlocked,
    None,
};

class UFortConversationTaskNode_ApplyAbility : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    UClass* GetGrantedAbility() const;

    int32 GetLevel() const;

    bool GetbActivateImmediatelyOnGranting() const;

    FName GetPricingKey() const;

    EPreventAbilityUseReason CheckPlayerFacingUsageRequirements(const FConversationContext& Context, const AFortPlayerPawn& PlayerPawn, int32 Price) const;

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;
};
