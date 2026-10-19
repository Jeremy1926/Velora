#include "pch.h"
#include "Internationalization/Text.h"
#include "Misc/EngineVersion.h"
#include "Templates/UnrealTemplate.h"
#include "Scanner.h"

static Scanner::FCode ClientBuild()
{
    static auto ClientBuild = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerCreateBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();

    return ClientBuild;
}

static Scanner::FCode UnableToAfford()
{
    static auto UnableToAfford = ClientBuild().Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*").Find("lea rdx, [0x*]");

    return UnableToAfford;
}

FText::FText()
{
    static auto Constructor = Scanner::Function<void (*)(FText*)>(Scanner::Pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 10 48 89 13 48 8B 48 08 48 8B C3 48 89 4B 08 48 85 C9 74 04 F0 FF 41 08 C7 43 10 00 00 00 00 48 83 C4 20 5B C3").Get(TEXT("FText::FText")));

    Constructor(this);
}

FText::FText(const FText& Source)
    : FText()
{
    *this = Source;
}

FText& FText::operator=(const FText& Source)
{
    static auto CopyAssign = Scanner::Function<FText& (*)(FText*, const FText&)>(ClientBuild().Find("lea rdx, [0x*]; lea rcx, *; call 0x*").Call().Get(TEXT("FText::operator=(const FText&)")));

    return CopyAssign(this, Source);
}

FText& FText::operator=(FText&& Source)
{
    static auto MoveAssign = Scanner::Function<FText& (*)(FText*, FText&)>(Scanner::Pattern("48 8B 02 4C 8B 01 48 89 01 4C 89 02 48 8B 42 08 4C 8B 41 08 48 89 41 08 4C 89 42 08 8B 42 10 89 41 10 48 8B C1 C3").Get(TEXT("FText::operator=(FText&&)")));

    return MoveAssign(this, Source);
}

bool FText::IsEmpty() const
{
    static auto IsEmpty = Scanner::Function<bool (*)(const FText*)>(ClientBuild().Find("mov r8d, *; *; call [*+0x*]; mov rdx, rax").Find("lea rcx, *; call 0x*", 2).Call().Get(TEXT("FText::IsEmpty")));

    return IsEmpty(this);
}

FText FText::Format(FTextFormat Fmt, FFormatOrderedArguments&& InArguments)
{
    static auto Format = Scanner::Function<FText (*)(FTextFormat, FFormatOrderedArguments&&)>(UnableToAfford().Find("mov r8, rax; lea rdx, *; lea rcx, *; call 0x*").Call().Get(TEXT("FText::Format")));

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

    static auto Format = Scanner::Function<FText (*)(FTextFormat, FFormatArgumentValue)>(UnableToAfford().Call(2).Get(TEXT("FText::Format")));

    return Format(Fmt, v1);
}

FTextFormat::FTextFormat(const FText& InText)
{
    static auto Constructor = Scanner::Function<void (*)(FTextFormat*, const FText&)>(UnableToAfford().Call().Get(TEXT("FTextFormat::FTextFormat")));

    Constructor(this, InText);
}

FFormatArgumentValue::FFormatArgumentValue(const FText& Value)
{
    static auto Constructor = []
    {
        auto Call = UnableToAfford().Last("call 0x*");

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
