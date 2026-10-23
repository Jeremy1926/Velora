#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"
#include "GameplayAbilitySpec.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include <utility>
#include <vector>

class AFortPlayerController;
class AFortPlayerPawn;

enum class EPreventUseStormCircleServiceReason : uint8
{
    CannotAfford,
    AlreadyActive,
    StormLocationsAlreadyRevealed,
    None,
};

class UFortConversationTaskNode_ShowFutureStormCircle : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    UClass* GetGrantedAbility() const;

    int32 GetLevel() const;

    EPreventUseStormCircleServiceReason CheckPlayerFacingUsageRequirements(const FConversationContext& Context, const AFortPlayerPawn& PlayerPawn, int32 Price) const;

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;

    static void HandleSafeZoneUpdated();

private:
    static std::vector<std::pair<TWeakObjectPtr<AFortPlayerController>, FGameplayAbilitySpecHandle>>& GetForecastPlayers();
};
