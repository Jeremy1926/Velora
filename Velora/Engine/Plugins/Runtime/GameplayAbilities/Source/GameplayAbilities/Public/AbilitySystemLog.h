#pragma once

#include "Logging/LogMacros.h"

extern FLogCategoryBase& LogAbilitySystem;

#define ABILITY_LOG(Verbosity, Format, ...) \
{ \
    UE_LOG(LogAbilitySystem, Verbosity, Format, ##__VA_ARGS__); \
}
