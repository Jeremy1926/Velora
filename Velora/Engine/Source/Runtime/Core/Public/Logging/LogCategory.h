#pragma once

#include "HAL/Platform.h"
#include "Logging/LogVerbosity.h"
#include "UObject/NameTypes.h"

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
        return CategoryFName;
    }

protected:
    ELogVerbosity::Type Verbosity;
    bool DebugBreakOnLog;
    uint8 DefaultVerbosity;
    ELogVerbosity::Type CompileTimeVerbosity;
    FName CategoryFName;
};
