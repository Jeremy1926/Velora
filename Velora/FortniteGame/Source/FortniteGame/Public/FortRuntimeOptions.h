#pragma once

#include "UObject/Object.h"

class UFortRuntimeOptions : public UObject
{
public:
    static UClass* StaticClass();

    static const UFortRuntimeOptions& Get();

    float GetBRServerMaxTickRate() const;

    bool GetDisableReplicationGraph() const;

    void SetEnableManualBroadcasterStart(bool bInEnableManualBroadcasterStart);
};
