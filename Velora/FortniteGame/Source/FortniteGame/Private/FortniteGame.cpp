#include "pch.h"
#include "FortniteGame.h"
#include "Scanner.h"

FLogCategoryBase& LogFort = *Anchor::Data<FLogCategoryBase*>(TEXT("LogFort"))
    .String(TEXT("SpawnDefaultPawnFor: Failed due to no world"))
    .Start()
    .Find("cmp byte ptr [0x*], *")
    .Target()
    .Get();

FLogCategoryBase& LogFortBuilding = *Anchor::Data<FLogCategoryBase*>(TEXT("LogFortBuilding"))
    .String(TEXT("LogFortBuilding"))
    .Find("lea rcx, [0x*]")
    .Target()
    .Get();

FLogCategoryBase& LogFortLoot = *Anchor::Data<FLogCategoryBase*>(TEXT("LogFortLoot"))
    .String(TEXT("LogFortLoot"))
    .Find("lea rcx, [0x*]")
    .Target()
    .Get();
