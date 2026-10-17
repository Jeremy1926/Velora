#pragma once

#include "HAL/Platform.h"
#include "Misc/EngineVersionBase.h"
#include "Containers/UnrealString.h"

class FEngineVersion : public FEngineVersionBase
{
public:
    static const FEngineVersion& Current();

private:
    FString Branch;
};
