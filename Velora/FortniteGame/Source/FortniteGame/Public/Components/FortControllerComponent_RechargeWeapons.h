#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Misc/Guid.h"
#include "UObject/SoftObjectPtr.h"

class UFortWorldItemDefinition;
class UScriptStruct;
class UWorld;

struct FCachedRechargeAmmoData
{
    static UScriptStruct* StaticStruct();

    float GetServerStartTime() const;

    float GetChargeRate() const;

    int32 GetAmountToRecharge() const;

    const TSoftObjectPtr<UFortWorldItemDefinition>& GetAmmoItemDefinition() const;

    bool GetShouldRechargeAmmoToClip() const;
};

class UFortControllerComponent_RechargeWeapons : public UActorComponent
{
public:
    static UClass* StaticClass();

    static void TickRechargingWeapons(UWorld* World, float DeltaSeconds);

    void TickComponent(float DeltaTime);

    void HandleRechargeAmmo(const FGuid& ItemGuid);

    void GetAmmoStackInfo(const FGuid& ItemGuid, int32& OutMaxAmount, int32& OutCurrentAmount);

    bool UpdateServerStartForItem(const FGuid& ItemGuid, bool bForce);

    TArray<TPair<FGuid, FCachedRechargeAmmoData*>> GetWeaponToRechargeDataMap();

    bool GetPreventRecharging() const;
};
