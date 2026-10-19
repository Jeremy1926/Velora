#include "pch.h"
#include "Player/StatManager.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

void UStat::ModifyStat(double Amount, EStatMod ModType)
{
    static auto ModifyStat = Scanner::CallTo(Scanner::Event(TEXT("ClientOnPawnRevived")).Get()).Find("mov r8b, 0x*").Call(2).Get(TEXT("UStat::ModifyStat"));

    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 19)
        Scanner::Function<void (*)(UStat*, double, EStatMod)>(ModifyStat)(this, Amount, ModType);
    else
        Scanner::Function<void (*)(UStat*, int32, EStatMod)>(ModifyStat)(this, (int32)Amount, ModType);
}

UStat* UStatManager::AddOrGetStat(FName StatName, EStatRecordingPeriod HighestPeriodToTrack)
{
    static auto AddOrGetStat = Scanner::Function<UStat* (*)(UStatManager*, FName, EStatRecordingPeriod)>(Scanner::CallTo(Scanner::Event(TEXT("ClientOnPawnRevived")).Get()).Find("mov r8b, 0x*").Call().Get(TEXT("UStatManager::AddOrGetStat")));

    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() == 4 && Version.GetMinor() < 21 && HighestPeriodToTrack > EStatRecordingPeriod::AthenaSafeZonePhase)
        HighestPeriodToTrack = (EStatRecordingPeriod)((uint8)HighestPeriodToTrack - 1);

    return AddOrGetStat(this, StatName, HighestPeriodToTrack);
}
