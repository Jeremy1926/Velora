#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "UObject/SoftObjectPtr.h"

class AController;
class AFortPlayerStateAthena;
class UFortQuestItemDefinition;
struct FGameplayTagQuery;

enum class EPlayerBountyThreatLevel : uint8
{
    Low,
    Medium,
    High,
    MAX
};

class UFortControllerComponent_TransientQuests : public UActorComponent
{
public:
    static UClass* StaticClass();

    void GrantTransientQuest(const UFortQuestItemDefinition* QuestDefToGrant);

    void GrantTransientQuest(TSoftObjectPtr<UFortQuestItemDefinition> QuestDefToGrantPtr);

    void OnTransientQuestGranted(const UFortQuestItemDefinition* GrantedQuestDef);

    void StartPlayerBountyThreatLevelUpdates(float MediumThreatRange, float HighThreatRange, float ThreatRefreshRate);

    void StopPlayerBountyThreatLevelUpdates();

    void UpdatePlayerBountyThreatLevel(float MediumThreatRange, float HighThreatRange);

    void ClientBroadcastOnPlayerBountyThreatLevelUpdated(EPlayerBountyThreatLevel NewPlayerBountyThreatLevel);

    static void TickPlayerBountyThreatLevelUpdates();

    static void GetPlayerBountyTargets(const AController* Hunter, const FGameplayTagQuery& TargetFilter, TArray<AFortPlayerStateAthena*>& OutHumanTargets, TArray<AFortPlayerStateAthena*>& OutBotTargets);

    AFortPlayerStateAthena*& GetTrackedHunterBountyTarget();

    AFortPlayerStateAthena*& GetTrackedProtectorBountyTarget();

    TArray<AFortPlayerStateAthena*>& GetTrackedBountyHunters();
};
