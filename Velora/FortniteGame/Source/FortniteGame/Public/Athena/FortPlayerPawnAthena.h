#pragma once

#include "Pawns/FortPlayerPawn.h"

struct FFortPickupRequestInfo;
struct FGameplayEffectContextHandle;
struct FGameplayTagContainer;
struct FGuid;
enum class EDeathCause : uint8;

class AFortPlayerPawnAthena : public AFortPlayerPawn
{
public:
    static UClass* StaticClass();

    void OnPlayerPawnReady();

    void ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    void ServerHandlePickupWithSwap_Implementation(AFortPickup* Pickup, const FGuid& Swap, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    void ServerHandlePickupWithRequestedSwap_Implementation(AFortPickup* Pickup, const FGuid& Swap, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    void ServerHandlePickupInfo_Implementation(AFortPickup* Pickup, const FFortPickupRequestInfo& Params);

    void ServerReviveFromDBNO_Implementation(AController* EventInstigator);

    void ForceReviveFromDBNO(AController* EventInstigator);

    void execForceReviveFromDBNO(FFrame& Stack, void* const Z_Param__Result);

    EDeathCause ToDeathCause(const FGameplayTagContainer& InTags, bool bWasDBNO);

    void InitializeDeathInfoOnPawnDeath(const FGameplayTagContainer& InTags, AController* EventInstigator, AController* DBNOFinisher, AActor* DamageCauser);

    void HandleDeath(float Damage, const FGameplayTagContainer& DamageTags, const FGameplayEffectContextHandle& EffectContext, AController* InstigatedBy, AActor* DamageCauser);

    float GetPickupSpeedMultiplier() const;

    uint8* GetDBNORevivalStacking();
};
