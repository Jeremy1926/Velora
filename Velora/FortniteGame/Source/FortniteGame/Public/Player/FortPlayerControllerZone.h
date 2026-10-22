#pragma once

#include "Player/FortPlayerControllerGameplay.h"
#include "Delegates/Delegate.h"

class AActor;
class AController;
class AFortPawn;
class AFortPlayerPawn;
class AFortPlayerState;
class APawn;
class UFortWeaponItemDefinition;
class UScriptStruct;
struct FGameplayEffectContextHandle;
struct FGameplayTagContainer;

struct FFortPlayerDeathReport
{
    static UScriptStruct* StaticStruct();

    AFortPlayerState* GetKillerPlayerState() const;

    AFortPawn* GetKillerPawn() const;

    UFortWeaponItemDefinition* GetKillerWeapon() const;
};

class AFortPlayerControllerZone : public AFortPlayerControllerGameplay
{
public:
    typedef AFortPlayerControllerGameplay Super;

    static UClass* StaticClass();

    FSimpleMulticastDelegate& GetOnPlayerPlaceChanged();

    void ServerAcknowledgePossession_Implementation(APawn* P);

    void ServerRestartPlayer_Implementation();

    void OnPawnDied(AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher);

    void OnPawnRevived(AController* EventInstigator);

    void ApplyDeathDurabilityPenalty(float DurabilityPenalty);

    FFortPlayerDeathReport& GetPlayerDeathReport();
};
