#pragma once

#include "HAL/Platform.h"
#include "Logging/LogCategory.h"
#include "Logging/LogVerbosity.h"

struct FMsg
{
    static void (*Logf_Internal)(const ANSICHAR* File, int32 Line, const FName& Category, ELogVerbosity::Type Verbosity, const TCHAR* Fmt, ...);
};

#define UE_LOG(CategoryName, Verbosity, Format, ...) \
    { \
        if (!CategoryName.IsSuppressed(ELogVerbosity::Verbosity)) \
            FMsg::Logf_Internal(__FILE__, __LINE__, CategoryName.GetCategoryName(), ELogVerbosity::Verbosity, Format, ##__VA_ARGS__); \
    }
