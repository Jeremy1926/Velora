#include "pch.h"
#include "FortGameModeZone.h"
#include "Items/FortInventory.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto FinishWorldInitialization = []
{
    auto AthenaFinishWorldInitialization = Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start().Get();
    auto AthenaVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
    auto Slot = Scanner::Slot(TEXT("AFortGameMode::FinishWorldInitialization"), AthenaVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == AthenaFinishWorldInitialization; });
    auto PvPBaseFinishWorldInitialization = Scanner::Virtual<const void*>(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGamePvPBase")), Slot);
    auto Call = Scanner::At(PvPBaseFinishWorldInitialization).Find("call 0x*");

    if (!Call.Get())
        Call = Scanner::At(AthenaFinishWorldInitialization).Find(std::format("call 0x{:x}", (uintptr_t)PvPBaseFinishWorldInitialization));

    return DetourCall(TEXT("AFortGameModeZone::FinishWorldInitialization"), Call.Get(), &AFortGameModeZone::FinishWorldInitialization);
}();

void AFortGameModeZone::FinishWorldInitialization()
{
    Super::FinishWorldInitialization();
}

TArray<FItemAndCount>& AFortGameModeZone::GetStartingItems()
{
    static auto StartingItems = GetClass()->FindPropertyByName(FName(TEXT("StartingItems")));

    return *StartingItems->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this);
}
