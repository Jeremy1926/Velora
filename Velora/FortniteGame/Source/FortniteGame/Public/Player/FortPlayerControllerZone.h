#pragma once

#include "Player/FortPlayerControllerGameplay.h"

class AActor;
class AController;
class AFortPlayerPawn;
class APawn;
struct FGameplayEffectContextHandle;
struct FGameplayTagContainer;

class AFortPlayerControllerZone : public AFortPlayerControllerGameplay
{
public:
    typedef AFortPlayerControllerGameplay Super;

    static UClass* StaticClass();

    void ServerAcknowledgePossession_Implementation(APawn* P);

    void ServerRestartPlayer_Implementation();

    void OnPawnDied(AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher);

    void OnPawnRevived(AController* EventInstigator);

    void ApplyDeathDurabilityPenalty(float DurabilityPenalty);
};
