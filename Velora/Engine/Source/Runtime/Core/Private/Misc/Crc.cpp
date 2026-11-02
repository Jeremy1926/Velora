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
        .Or()
        .Pattern("4C 8B D2 85 C9 7E ? 48 89 1C 24 48 8D 1D ? ? ? ? 44 8B D9")
        .Start()
        .Or()
        .Pattern("4C 8B D2 85 C9 7F ? 48 83 C4 08 C3 48 89 1C 24 48 8D 1D ? ? ? ? 44 8B D9")
        .Start()
        .Get();

    static auto bLength = Anchor::Exists()
        .At(Strihash_DEPRECATED)
        .Has("test ecx, ecx; j*le 0x*")
        .Get();

    if (bLength)
        return ((uint32 (*)(const int32, const TCHAR*))Strihash_DEPRECATED)((int32)wcslen(Data), Data);

    return ((uint32 (*)(const TCHAR*))Strihash_DEPRECATED)(Data);
}

uint32 FCrc::MemCrc32(const void* Data, int32 Length, uint32 CRC)
{
    static auto MemCrc32 = Anchor::Function<uint32 (*)(const void*, int32, uint32)>(TEXT("FCrc::MemCrc32"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movaps xmm2, xmm0; lea rdx, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("lea rcx, [rdi+0x*]; movaps xmm2, xmm0; lea rdx, [rsp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rax, [0x*]")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 45 8B D0 48 8D 35 ? ? ? ? 44 8D 41 03 8B DA 41 83 E0 FC 4C 8B D9 44 2B C1 41 F7 D2 41 3B D0 7E ? 41 2B D8 45 85 C0 0F 85 ? ? ? ? 8B C3 B9 08 00 00 00 99 F7 F9")
        .Get();

    return MemCrc32(Data, Length, CRC);
}
