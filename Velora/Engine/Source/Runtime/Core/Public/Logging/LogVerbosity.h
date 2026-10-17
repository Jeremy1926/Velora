#pragma once

#include "HAL/Platform.h"

namespace ELogVerbosity
{
    enum Type : uint8
    {
        NoLogging = 0,
        Fatal,
        Error,
        Warning,
        Display,
        Log,
        Verbose,
        VeryVerbose,
        All = VeryVerbose,
        NumVerbosity,
        VerbosityMask = 0xf,
        SetColor = 0x40,
        BreakOnLog = 0x80
    };
}
