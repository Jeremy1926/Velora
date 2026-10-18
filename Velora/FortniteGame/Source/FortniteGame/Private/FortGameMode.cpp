#include "pch.h"
#include "FortGameMode.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void AFortGameMode::FinishWorldInitialization()
{
    static auto FinishWorldInitialization = []
    {
        auto AthenaFinishWorldInitialization = Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start().Get();
        auto AthenaVTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (AthenaVTable[Index] != AthenaFinishWorldInitialization)
            Index++;

        auto VTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameMode"));

        return (void (*)(AFortGameMode*))VTable[Index];
    }();

    FinishWorldInitialization(this);
}

void AFortGameMode::OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions)
{
    static auto OnGivenMatchAssignment = []
    {
        auto AthenaOnGivenMatchAssignment = Scanner::String(TEXT("match.option.warmupRequiredPlayerCount")).Start().Get();
        auto AthenaVTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (AthenaVTable[Index] != AthenaOnGivenMatchAssignment)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortGameMode*, const TMap<FString, FString>&))(VTable + OnGivenMatchAssignment))(this, MatchOptions);
}
