#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Engine/EngineBaseTypes.h"
#include "Misc/Guid.h"
#include "UObject/SoftObjectPtr.h"

class UFortWorldItem;
class UFortWorldItemDefinition;
class UScriptStruct;
class UWorld;
struct FActorComponentTickFunction;

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

    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

    void OnItemInstanceAdded(UFortWorldItem* Item);

    void OnItemInstanceRemoved(UFortWorldItem* Item);

    void OnWeaponConsumedAmmo(const FGuid& ItemGuid);

    void AttemptSetTickEnabledForItem(const FGuid& ItemGuid);

    void HandleRechargeAmmo(const FGuid& ItemGuid);

    void GetAmmoStackInfo(const FGuid& ItemGuid, int32& OutMaxAmount, int32& OutCurrentAmount);

    bool UpdateServerStartForItem(const FGuid& ItemGuid, bool bForce);

    TArray<TPair<FGuid, FCachedRechargeAmmoData*>> GetWeaponToRechargeDataMap();

    FCachedRechargeAmmoData* FindRechargeDataForItem(const FGuid& ItemGuid);

    bool GetPreventRecharging() const;
};
