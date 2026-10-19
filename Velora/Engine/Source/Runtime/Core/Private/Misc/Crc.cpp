#include "pch.h"
#include "Misc/Crc.h"
#include "Scanner.h"

uint32 FCrc::Strihash_DEPRECATED(const TCHAR* Data)
{
    static auto Strihash_DEPRECATED = Scanner::Function<uint32 (*)(const TCHAR*)>([]
    {
        auto Option = Scanner::String(TEXT("match.option.warmupRequiredPlayerCount"));

        return (Option.Get() ? Option : Scanner::String(TEXT("match.option.customKey"))).Call(2).Call().Get(TEXT("FCrc::Strihash_DEPRECATED"));
    }());

    return Strihash_DEPRECATED(Data);
}
