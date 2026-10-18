#include "pch.h"
#include "Player/StatManager.h"
#include "Scanner.h"

void UStat::ModifyStat(int32 Amount, EStatMod ModType)
{
    static auto ModifyStat = Scanner::Function<void (*)(UStat*, int32, EStatMod)>(Scanner::CallTo(Scanner::Event(TEXT("ClientOnPawnRevived")).Get()).Find("mov r8b, 0x3").Call(2).Get(TEXT("UStat::ModifyStat")));

    ModifyStat(this, Amount, ModType);
}

UStat* UStatManager::AddOrGetStat(FName StatName, EStatRecordingPeriod HighestPeriodToTrack)
{
    static auto AddOrGetStat = Scanner::Function<UStat* (*)(UStatManager*, FName, EStatRecordingPeriod)>(Scanner::CallTo(Scanner::Event(TEXT("ClientOnPawnRevived")).Get()).Find("mov r8b, 0x3").Call().Get(TEXT("UStatManager::AddOrGetStat")));

    return AddOrGetStat(this, StatName, HighestPeriodToTrack);
}
