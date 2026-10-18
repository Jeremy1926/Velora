#pragma once

#include "Pawns/FortPawn.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"

class AController;
class AFortPickup;
class UGameplayEffect;
struct FGameplayTag;
struct FScalableFloat;

class AFortPlayerPawn : public AFortPawn
{
public:
    void OnBlueprintPlace();

    void ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    void ServerReviveFromDBNO_Implementation(AController* EventInstigator);

    bool CanReviveFromDBNO(AController* EventInstigator);

    void UpdateDBNOInteractCollision();

    TSubclassOf<UGameplayEffect>& GetSelfReviveGameplayEffect();

    TSubclassOf<UGameplayEffect>& GetTeammateReviveGameplayEffect();

    FScalableFloat& GetSetByCallerReviveHealth();

    FGameplayTag& GetEventReviveTag();
};
