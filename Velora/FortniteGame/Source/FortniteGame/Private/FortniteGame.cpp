#include "pch.h"
#include "FortniteGame.h"
#include "Scanner.h"

FLogCategoryBase& LogFort = *(FLogCategoryBase*)Scanner::String(TEXT("SpawnDefaultPawnFor: Failed due to no world")).Start().Find("cmp byte ptr [0x*], *").Target();

FLogCategoryBase& LogFortBuilding = *(FLogCategoryBase*)Scanner::String(TEXT("LogFortBuilding")).Find("lea rcx, [0x*]").Target();

FLogCategoryBase& LogFortLoot = *(FLogCategoryBase*)Scanner::String(TEXT("LogFortLoot")).Find("lea rcx, [0x*]").Target();
