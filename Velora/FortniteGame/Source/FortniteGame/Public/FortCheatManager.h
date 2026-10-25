#pragma once

#include "GameFramework/CheatManager.h"

class FOutputDevice;

class UFortCheatManager : public UCheatManager
{
public:
    static UClass* StaticClass();

    bool ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor);
};
