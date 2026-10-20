#pragma once

#include "UObject/Object.h"
#include "InputCoreTypes.h"

class UInputSettings : public UObject
{
public:
    static UInputSettings* GetInputSettings();

    TArray<FKey>& GetConsoleKeys();
};
