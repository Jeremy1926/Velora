#pragma once

#include "Building/BuildingActor.h"
#include "Containers/Array.h"

class AActor;
class AFortPlayerPawn;
class AFortPlayerStateAthena;
struct FFrame;
struct FGameplayTagQuery;

class ABuildingGameplayActorTransientQuest : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void StartUpdatingInteractionText(const AActor* TriggeredActor);

    void StopUpdatingInteractionText(const AActor* TriggeredActor);

    void ServerUpdateInteractionText_Implementation();

    void MulticastSetInteractionText(const AFortPlayerPawn* PawnInTriggerVolume, bool bBountyAvailable, bool bBountyInProgress);

    void OnBountyStarted(const TArray<AFortPlayerStateAthena*>& HunterPlayerStates);

    void execStartUpdatingInteractionText(FFrame& Stack, void* const Z_Param__Result);

    void execStopUpdatingInteractionText(FFrame& Stack, void* const Z_Param__Result);

    static void TickInteractionTextUpdates();

    TArray<AFortPlayerPawn*>& GetPawnsInTriggerVolume();

    float GetInteractionTextRefreshRate() const;

    const FGameplayTagQuery& GetTargetFilter() const;
};
