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

    static auto Constructor = Scanner::Function<void (*)(FName*, EName)>(Scanner::String(TEXT("RecordReplay: failed to create demo net driver!")).Last("mov edx, 0x11e").Find("call 0x*").Target(TEXT("FName::FName(EName)")));

    Constructor(this, N);
}

FName::FName(const WIDECHAR* Name, EFindName FindType)
{
    static auto Constructor = Scanner::Function<void (*)(FName*, const WIDECHAR*, EFindName)>(Scanner::String(TEXT("Illegal call to StaticFindObject() while collecting garbage!")).Start().Find("mov r8d, 0x1").Call().Get(TEXT("FName::FName")));

    Constructor(this, Name, FindType);
}

FString FName::ToString() const
{
    static auto ToString = Scanner::Function<void (*)(const FName*, FString&)>(Scanner::String(TEXT("Failed to find function %s in %s")).Start().Call(3).Get(TEXT("FName::ToString")));

    FString Out;

    ToString(this, Out);

    return Out;
}
