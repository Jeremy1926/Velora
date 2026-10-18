#pragma once

#include "HAL/Platform.h"
#include "Misc/EngineVersionBase.h"
#include "Containers/UnrealString.h"

class FEngineVersion : public FEngineVersionBase
{
public:
    static const FEngineVersion& Current();

    FString GetBranch() const
    {
        return Branch;
    }

private:
    FString Branch;
};
