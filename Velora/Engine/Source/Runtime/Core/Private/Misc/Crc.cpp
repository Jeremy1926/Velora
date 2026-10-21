#include "pch.h"
#include "Misc/Crc.h"
#include "Scanner.h"

uint32 FCrc::Strihash_DEPRECATED(const TCHAR* Data)
{
    static auto Strihash_DEPRECATED = Anchor::Function<const void*>(TEXT("FCrc::Strihash_DEPRECATED"))
        .String(TEXT("match.option.warmupRequiredPlayerCount"))
        .Call(2)
        .Call()
        .CallUntil("shr *, 0x8")
        .Or()
        .String(TEXT("match.option.customKey"))
        .Call(2)
        .Call()
        .CallUntil("shr *, 0x8")
        .Get();

    static auto bLength = Anchor::Exists()
        .At(Strihash_DEPRECATED)
        .Has("test ecx, ecx; jle 0x*")
        .Get();

    if (bLength)
        return ((uint32 (*)(const int32, const TCHAR*))Strihash_DEPRECATED)((int32)wcslen(Data), Data);

    return ((uint32 (*)(const TCHAR*))Strihash_DEPRECATED)(Data);
}
