#include "pch.h"
#include "Player/StatManager.h"
#include "Misc/EngineVersion.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UStat::ModifyStat(double Amount, EStatMod ModType)
{
    static auto ModifyStat = Anchor::Function<const void*>(TEXT("UStat::ModifyStat"))
        .Event(TEXT("ClientOnPawnRevived"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Find("mov r8b, 0x*")
        .Call(2)
        .Get();

    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 19)
        ((void (*)(UStat*, double, EStatMod))ModifyStat)(this, Amount, ModType);
    else
        ((void (*)(UStat*, int32, EStatMod))ModifyStat)(this, (int32)Amount, ModType);
}

UStat* UStatManager::AddOrGetStat(FName StatName, EStatRecordingPeriod HighestPeriodToTrack)
{
    static auto AddOrGetStat = Anchor::Function<UStat* (*)(UStatManager*, FName, EStatRecordingPeriod)>(TEXT("UStatManager::AddOrGetStat"))
        .Event(TEXT("ClientOnPawnRevived"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Find("mov r8b, 0x*")
        .Call()
        .Get();

    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() == 4 && Version.GetMinor() < 21 && HighestPeriodToTrack > EStatRecordingPeriod::AthenaSafeZonePhase)
        HighestPeriodToTrack = (EStatRecordingPeriod)((uint8)HighestPeriodToTrack - 1);

    return AddOrGetStat(this, StatName, HighestPeriodToTrack);
}
