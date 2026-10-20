#pragma once

#include "UObject/Object.h"
#include "Containers/UnrealString.h"

class UFortGlobals : public UObject
{
public:
    static UClass* StaticClass();

    static UFortGlobals* Get();

    FString* GetBRMapFullName();
};
