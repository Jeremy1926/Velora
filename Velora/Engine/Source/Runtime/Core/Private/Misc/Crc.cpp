#include "pch.h"
#include "Misc/Crc.h"
#include "Scanner.h"

uint32 FCrc::Strihash_DEPRECATED(const TCHAR* Data)
{
    static auto Strihash_DEPRECATED = (uint32 (*)(const TCHAR*))Scanner::String(TEXT("match.option.warmupRequiredPlayerCount")).Call(2).Call().Get();

    return Strihash_DEPRECATED(Data);
}
