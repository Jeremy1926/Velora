#pragma once

#include "HAL/Platform.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UAbilitySystemComponent;
class UScriptStruct;

struct alignas(8) FPredictionKey
{
    typedef int16 KeyType;

    FPredictionKey();

    static UScriptStruct* StaticStruct();

    KeyType GetCurrent() const;

    bool IsValidKey() const
    {
        return GetCurrent() > 0;
    }

private:
    uint8 Data[0x18];
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
