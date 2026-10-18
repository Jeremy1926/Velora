#include "pch.h"
#include "FortGameModeZone.h"
#include "Items/FortInventory.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

static auto FinishWorldInitialization = DetourCall(TEXT("AFortGameModeZone::FinishWorldInitialization"), Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start().Call().Find("call 0x*").Get(), &AFortGameModeZone::FinishWorldInitialization);

void AFortGameModeZone::FinishWorldInitialization()
{
    Super::FinishWorldInitialization();
}

TArray<FItemAndCount>& AFortGameModeZone::GetStartingItems()
{
    static auto StartingItems = GetClass()->FindPropertyByName(FName(TEXT("StartingItems")));

    return *StartingItems->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this);
}
