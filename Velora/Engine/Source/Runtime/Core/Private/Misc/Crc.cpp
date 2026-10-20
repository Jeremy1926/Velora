#include "pch.h"
#include "Misc/Crc.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

uint32 FCrc::Strihash_DEPRECATED(const TCHAR* Data)
{
    static auto Strihash_DEPRECATED = []
    {
        auto Option = Scanner::String(TEXT("match.option.warmupRequiredPlayerCount"));
        auto Strihash = (Option.Get() ? Option : Scanner::String(TEXT("match.option.customKey"))).Call(2).Call();

        while (Strihash.Get() && !Strihash.Has("shr *, 0x8"))
            Strihash = Strihash.Call();

        return Strihash.Get(TEXT("FCrc::Strihash_DEPRECATED"));
    }();

    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 25)
        return Scanner::Function<uint32 (*)(const int32, const TCHAR*)>(Strihash_DEPRECATED)((int32)wcslen(Data), Data);

    return Scanner::Function<uint32 (*)(const TCHAR*)>(Strihash_DEPRECATED)(Data);
}
