#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"
#include "GameplayAbilitySpec.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include <utility>
#include <vector>

class AFortPlayerController;

class UFortConversationTaskNode_ShowFutureStormCircle : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    UClass* GetGrantedAbility() const;

    int32 GetLevel() const;

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    static void HandleSafeZoneUpdated();

private:
    static std::vector<std::pair<TWeakObjectPtr<AFortPlayerController>, FGameplayAbilitySpecHandle>>& GetForecastPlayers();
};
