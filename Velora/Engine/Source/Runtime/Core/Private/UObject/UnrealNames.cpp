#include "pch.h"
#include "UObject/NameTypes.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

FName::FName(EName N)
{
    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() == 4 && Version.GetMinor() < 24)
    {
        ComparisonIndex = N;
        Number = 0;
        return;
    }

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 25)
    {
        static auto FromValidEName = Anchor::Function<void (*)(NAME_INDEX*, EName)>(TEXT("FNameEntryId::FromValidEName"))
            .String(TEXT("RecordReplay: failed to create demo net driver!"))
            .Last("mov edx, 0x11e")
            .Find("call 0x*")
            .Target()
            .Or()
            .Pattern("48 89 5C 24 08 57 48 83 EC 20 48 63 DA 48 8B F9 E8 ? ? ? ? 8B 84 98 ? ? ? ? 48 8B 5C 24 30 89 07 48 8B C7")
            .Or()
            .Pattern("48 89 5C 24 08 57 48 83 EC 20 8B DA 48 8B F9 E8 ? ? ? ? 8B 84 98 ? ? ? ? 48 8B 5C 24 30 89 07 48 8B C7")
            .Get();

        ComparisonIndex = NAME_None;
        Number = 0;

        if (N != NAME_None)
            FromValidEName(&ComparisonIndex, N);

        return;
    }

    static auto Constructor = Anchor::Function<void (*)(FName*, EName)>(TEXT("FName::FName(EName)"))
        .String(TEXT("RecordReplay: failed to create demo net driver!"))
        .Last("mov edx, 0x11e")
        .Find("call 0x*")
        .Target()
        .Get();

    Constructor(this, N);
}

FName::FName(const WIDECHAR* Name, EFindName FindType)
{
    static auto MakeDetectNumber = Anchor::Optional<FName* (*)(FName*, const void*, EFindName)>(TEXT("FNameHelper::MakeDetectNumber"))
        .String(TEXT("OnlineSubsystemUtils"), "lea rcx, [0x*]; lea rdx, [rsp+0x*]; mov [rsp+0x20], rcx; call 0x*; mov [rsp+0x28], eax")
        .Find("mov r8d, 0x1; movdqa [*], xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (MakeDetectNumber)
    {
        struct
        {
            const WIDECHAR* Data;
            int32 Len;
            bool bIsWide;
        } View { Name, 0, false };

        uint32 Chars = 0;

        while (Name && Name[View.Len])
            Chars |= Name[View.Len++];

        View.bIsWide = (Chars & 0xFFFFFF80) != 0;

        MakeDetectNumber(this, &View, FindType);

        return;
    }

    static auto Constructor = Anchor::Function<void (*)(FName*, const WIDECHAR*, EFindName)>(TEXT("FName::FName"))
        .String(TEXT("OnlineSubsystemUtils"), "lea rdx, [0x*]; lea rcx, [0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"))
        .Start()
        .Find("mov r8d, 0x1")
        .Call()
        .Get();

    Constructor(this, Name, FindType);
}

uint32 GetTypeHash(const FName N)
{
    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() == 4 && Version.GetMinor() < 24)
        return N.ComparisonIndex + N.Number;

    const uint32 Block = N.ComparisonIndex >> 16;
    const uint32 Offset = N.ComparisonIndex & 0xFFFF;

    return (Block << 19) + Block + (Offset << 16) + Offset + (Offset >> 4) + N.Number;
}

FString FName::ToString() const
{
    static auto ToString = Anchor::Function<void (*)(const FName*, FString&)>(TEXT("FName::ToString"))
        .String(TEXT("Failed to find function %s in %s"))
        .Is("call 0x*")
        .Last("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Failed to find function %s in %s"))
        .Start()
        .Call(3)
        .Get();

    FString Out;

    ToString(this, Out);

    return Out;
}
