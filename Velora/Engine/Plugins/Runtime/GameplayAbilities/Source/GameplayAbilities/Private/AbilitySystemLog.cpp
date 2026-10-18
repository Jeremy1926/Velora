#include "pch.h"
#include "AbilitySystemLog.h"
#include "Scanner.h"

FLogCategoryBase& LogAbilitySystem = *(FLogCategoryBase*)Scanner::String(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!")).Start().Find("cmp byte ptr [0x*], *").Target(TEXT("LogAbilitySystem"));
