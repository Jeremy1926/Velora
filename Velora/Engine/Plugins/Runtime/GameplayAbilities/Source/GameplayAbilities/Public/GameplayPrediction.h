#pragma once

#include "HAL/Platform.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UAbilitySystemComponent;
class UPackageMap;

struct FPredictionKey
{
    typedef int16 KeyType;

    FPredictionKey()
        : Current(0), Base(0), PredictiveConnection(nullptr), bIsStale(false), bIsServerInitiated(false)
    {
    }

    int16 Current;
    int16 Base;
    UPackageMap* PredictiveConnection;
    bool bIsStale;
    bool bIsServerInitiated;

    bool IsValidKey() const
    {
        return Current > 0;
    }
};

struct FScopedPredictionWindow
{
    FScopedPredictionWindow(UAbilitySystemComponent* AbilitySystemComponent, FPredictionKey InPredictionKey, bool InSetReplicatedPredictionKey = true);

    ~FScopedPredictionWindow();

    FPredictionKey ScopedPredictionKey;

private:
    TWeakObjectPtr<UAbilitySystemComponent> Owner;
    bool ClearScopedPredictionKey;
    bool SetReplicatedPredictionKey;
    FPredictionKey RestoreKey;
};
