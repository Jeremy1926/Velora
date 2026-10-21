#pragma once

#include "Pawns/FortPlayerPawn.h"

struct FFortPickupRequestInfo;
struct FGuid;

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

    float GetPickupSpeedMultiplier() const;

    uint8& GetDBNORevivalStacking();
};
