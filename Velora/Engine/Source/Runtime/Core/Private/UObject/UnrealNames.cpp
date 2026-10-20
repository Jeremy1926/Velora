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
    static auto Constructor = Anchor::Function<void (*)(FName*, const WIDECHAR*, EFindName)>(TEXT("FName::FName"))
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
        .Start()
        .Call(3)
        .Get();

    FString Out;

    ToString(this, Out);

    return Out;
}
