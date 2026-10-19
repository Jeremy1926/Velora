#include "pch.h"
#include "Internationalization/Text.h"
#include "Scanner.h"

static Scanner::FCode ClientBuild()
{
    static auto ClientBuild = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerCreateBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();

    return ClientBuild;
}

static Scanner::FCode FormatResourceName()
{
    static auto FormatResourceName = ClientBuild().Find("call 0x*; movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*");

    return FormatResourceName;
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
    static auto CopyAssign = (FText& (*)(FText*, const FText&))ClientBuild().Find("lea rdx, [0x*]; lea rcx, *; call 0x*").Call().Get(TEXT("FText::operator=(const FText&)"));

    return CopyAssign(this, Source);
}

FText& FText::operator=(FText&& Source)
{
    static auto MoveAssign = (FText& (*)(FText*, FText&))FormatResourceName().Find("lea rdx, [0x*]").Call(3).Get(TEXT("FText::operator=(FText&&)"));

    return MoveAssign(this, Source);
}

bool FText::IsEmpty() const
{
    static auto IsEmpty = (bool (*)(const FText*))ClientBuild().Find("lea rcx, *; call 0x*; test al, al; jnz *; lea rdx, *; mov rcx, *; call 0x*").Call().Get(TEXT("FText::IsEmpty"));

    return IsEmpty(this);
}

FText FText::Format(FTextFormat Fmt, FFormatArgumentValue v1)
{
    static auto Format = (FText (*)(FTextFormat, FFormatArgumentValue))FormatResourceName().Find("lea rdx, [0x*]").Call(2).Get(TEXT("FText::Format"));

    return Format(Fmt, v1);
}

FTextFormat::FTextFormat(const FText& InText)
{
    static auto Constructor = (void (*)(FTextFormat*, const FText&))FormatResourceName().Find("lea rdx, [0x*]").Call().Get(TEXT("FTextFormat::FTextFormat"));

    Constructor(this, InText);
}

FFormatArgumentValue::FFormatArgumentValue(const FText& Value)
{
    static auto Constructor = (void (*)(FFormatArgumentValue*, const FText&))FormatResourceName().Call(4).Get(TEXT("FFormatArgumentValue::FFormatArgumentValue"));

    Constructor(this, Value);
}
