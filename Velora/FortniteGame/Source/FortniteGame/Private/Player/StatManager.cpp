#include "pch.h"
#include "Player/StatManager.h"
#include "Misc/EngineVersion.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static Scanner::FCode OnPawnRevived()
{
    static auto OnPawnRevived = []
    {
        auto ClientOnPawnRevived = Scanner::Event(TEXT("ClientOnPawnRevived")).Get();
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"));

        for (int32 Index = 0; Scanner::IsCode(VTable[Index]); Index++)
        {
            auto Entry = Scanner::At(VTable[Index]);
            auto Call = Entry.Find("call 0x*");

            for (int32 CallIndex = 2; Call.Get() && Call.Target() != ClientOnPawnRevived; CallIndex++)
                Call = Entry.Find("call 0x*", CallIndex);

            if (Call.Get())
                return Entry;
        }

        return Scanner::FCode();
    }();

    return OnPawnRevived;
}

void UStat::ModifyStat(double Amount, EStatMod ModType)
{
    static auto ModifyStat = OnPawnRevived().Find("mov r8b, 0x*").Call(2).Get(TEXT("UStat::ModifyStat"));

    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 19)
        Scanner::Function<void (*)(UStat*, double, EStatMod)>(ModifyStat)(this, Amount, ModType);
    else
        Scanner::Function<void (*)(UStat*, int32, EStatMod)>(ModifyStat)(this, (int32)Amount, ModType);
}

UStat* UStatManager::AddOrGetStat(FName StatName, EStatRecordingPeriod HighestPeriodToTrack)
{
    static auto AddOrGetStat = Scanner::Function<UStat* (*)(UStatManager*, FName, EStatRecordingPeriod)>(OnPawnRevived().Find("mov r8b, 0x*").Call().Get(TEXT("UStatManager::AddOrGetStat")));

    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() == 4 && Version.GetMinor() < 21 && HighestPeriodToTrack > EStatRecordingPeriod::AthenaSafeZonePhase)
        HighestPeriodToTrack = (EStatRecordingPeriod)((uint8)HighestPeriodToTrack - 1);

    return AddOrGetStat(this, StatName, HighestPeriodToTrack);
}
