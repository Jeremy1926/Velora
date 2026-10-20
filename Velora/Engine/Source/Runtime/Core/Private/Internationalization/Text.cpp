#include "pch.h"
#include "Internationalization/Text.h"
#include "Misc/EngineVersion.h"
#include "Templates/UnrealTemplate.h"
#include "Scanner.h"

FText::FText()
{
    static auto Constructor = Anchor::Function<void (*)(FText*)>(TEXT("FText::FText"))
        .Pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 10 48 89 13 48 8B 48 08 48 8B C3 48 89 4B 08 48 85 C9 74 04 F0 FF 41 08 C7 43 10 00 00 00 00 48 83 C4 20 5B C3")
        .Get();

    Constructor(this);
}

FText::FText(const FText& Source)
    : FText()
{
    *this = Source;
}

FText& FText::operator=(const FText& Source)
{
    static auto CopyAssign = Anchor::Function<FText& (*)(FText*, const FText&)>(TEXT("FText::operator=(const FText&)"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [0x*]; lea rcx, *; call 0x*")
        .Call()
        .Get();

    return CopyAssign(this, Source);
}

FText& FText::operator=(FText&& Source)
{
    static auto MoveAssign = []
    {
        auto MoveAssign = Scanner::Pattern("48 8B 02 4C 8B 01 48 89 01 4C 89 02 48 8B 42 08 4C 8B 41 08 48 89 41 08 4C 89 42 08 8B 42 10 89 41 10 48 8B C1 C3");

        return MoveAssign.Get() ? (FText& (*)(FText*, FText&))MoveAssign.Get(TEXT("FText::operator=(FText&&)")) : nullptr;
    }();

    if (MoveAssign)
        return MoveAssign(this, Source);

    TextData = MoveTemp(Source.TextData);
    Flags = Source.Flags;

    return *this;
}

bool FText::IsEmpty() const
{
    static auto IsEmpty = Anchor::Function<bool (*)(const FText*)>(TEXT("FText::IsEmpty"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("mov r8d, *; *; call [*+0x*]")
        .Is("mov r8d, *; *; call [*+0x*]; mov rdx, rax")
        .Find("lea rcx, *; call 0x*", 2)
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("mov r8d, *; *; call [*+0x*]")
        .Find("lea rcx, *; call 0x*")
        .Call()
        .Get();

    return IsEmpty(this);
}

FText FText::Format(FTextFormat Fmt, FFormatOrderedArguments&& InArguments)
{
    static auto Format = Anchor::Function<FText (*)(FTextFormat, FFormatOrderedArguments&&)>(TEXT("FText::Format"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Find("* r8, *; lea rdx, *; lea rcx, *; call 0x*")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Find("* r8, *; lea rdx, *; lea rcx, *; call 0x*")
        .Call()
        .Get();

    return Format(Fmt, MoveTemp(InArguments));
}

FText FText::Format(FTextFormat Fmt, FFormatArgumentValue v1)
{
    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 21)
    {
        FFormatOrderedArguments Arguments;
        new(Arguments) FFormatArgumentValue(v1);

        return Format(Fmt, MoveTemp(Arguments));
    }

    static auto Format = Anchor::Function<FText (*)(FTextFormat, FFormatArgumentValue)>(TEXT("FText::Format"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Call(2)
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Call(2)
        .Get();

    return Format(Fmt, v1);
}

FTextFormat::FTextFormat(const FText& InText)
{
    static auto Constructor = Anchor::Function<void (*)(FTextFormat*, const FText&)>(TEXT("FTextFormat::FTextFormat"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Call()
        .Get();

    Constructor(this, InText);
}

FFormatArgumentValue::FFormatArgumentValue(const FText& Value)
{
    static auto Constructor = []
    {
        auto ClientBuild = Scanner::CallTo(Scanner::Event(TEXT("ServerCreateBuildingActor")).Get()).Start();
        auto CheckAutoChangeMaterial = ClientBuild.Find("lea rdx, [rsp+0x*]; call 0x*; test al, al").Call();
        auto Call = (CheckAutoChangeMaterial.Get() ? CheckAutoChangeMaterial : ClientBuild).Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*").Find("lea rdx, [0x*]").Last("call 0x*");

        return Scanner::At(Call.Target()).Has("mov dword ptr [rcx], 0x4") ? (void (*)(FFormatArgumentValue*, const FText&))Call.Target(TEXT("FFormatArgumentValue::FFormatArgumentValue")) : nullptr;
    }();

    if (Constructor)
    {
        Constructor(this, Value);
        return;
    }

    Type = 4;
    new (&TextValue) FText(Value);
    bIsTextValueSet = true;
}
