#pragma once

#include "HAL/Platform.h"
#include "Logging/LogVerbosity.h"
#include "UObject/NameTypes.h"
#include "Misc/EngineVersion.h"

struct FLogCategoryBase
{
    FLogCategoryBase(const TCHAR* CategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity);

    FORCEINLINE bool IsSuppressed(ELogVerbosity::Type VerbosityLevel) const
    {
        if ((VerbosityLevel & ELogVerbosity::VerbosityMask) <= Verbosity)
            return false;

        return true;
    }

    FORCEINLINE FName GetCategoryName() const
    {
        auto& Version = FEngineVersion::Current();

        return Version.GetMajor() > 4 || Version.GetMinor() >= 22 ? *(const FName*)(&CompileTimeVerbosity + 1) : CategoryFName;
    }

protected:
    ELogVerbosity::Type Verbosity;
    bool DebugBreakOnLog;
    uint8 DefaultVerbosity;
    ELogVerbosity::Type CompileTimeVerbosity;
    FName CategoryFName;
};
