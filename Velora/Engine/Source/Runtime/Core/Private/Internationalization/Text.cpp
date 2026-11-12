#include "pch.h"
#include "Internationalization/Text.h"
#include "Containers/UnrealString.h"
#include "Misc/EngineVersion.h"
#include "HAL/UnrealMemory.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

int32 FText::GetTypeSize()
{
    static auto TypeSize = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.KismetTextLibrary:GetEmptyText")))->FindPropertyByName(FName(TEXT("ReturnValue")))->GetSize();

    return TypeSize;
}

bool FText::HasRefCountedTextData()
{
    return GetTypeSize() < (int32)sizeof(FText);
}

FText::FText()
    : TextData()
    , Flags(0)
{
    static auto Constructor = Anchor::Optional<void (*)(FText*)>(TEXT("FText::FText"))
        .Pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 10 48 89 13 48 8B 48 08 48 8B C3 48 89 4B 08 48 85 C9 74 04 F0 FF 41 08 C7 43 10 00 00 00 00 48 83 C4 20 5B C3")
        .Get();

    if (Constructor)
    {
        Constructor(this);

        return;
    }

    static auto GetEmpty = Anchor::Function<const FText& (*)()>(TEXT("FText::GetEmpty"))
        .Pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 10 48 89 13 48 8B 48 08")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/UMG.EditableTextBox:ClearError"))
        .Find("cmp rax, 0x1; sbb rax, 0xffffffffffffffff; mov [rdx+0x20], rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetTextLibrary:GetEmptyText"))
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetTextLibrary:GetEmptyText"))
        .Find("add [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    *this = GetEmpty();
    GetFlags() = 0;
}

FText::FText(const FText& Source)
    : FText()
{
    *this = Source;
}

FText::~FText()
{
    if (!HasRefCountedTextData())
    {
        ((TSharedRef<ITextData, ESPMode::ThreadSafe>*)TextData)->~TSharedRef();

        return;
    }

    if (auto Data = *(ITextData**)TextData)
        (*(void (***)(ITextData*))Data)[2](Data);
}

FText& FText::operator=(const FText& Source)
{
    static auto CopyAssign = Anchor::Optional<FText& (*)(FText*, const FText&)>(TEXT("FText::operator=(const FText&)"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [0x*]; lea rcx, *; call 0x*")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Find("lea rdx, [0x*]; lea rcx, *; call 0x*")
        .Call()
        .Get();

    if (CopyAssign)
        return CopyAssign(this, Source);

    *(TSharedRef<ITextData, ESPMode::ThreadSafe>*)TextData = *(const TSharedRef<ITextData, ESPMode::ThreadSafe>*)Source.TextData;
    Flags = Source.Flags;

    return *this;
}

FText& FText::operator=(FText&& Source)
{
    static auto MoveAssign = Anchor::Optional<FText& (*)(FText*, FText&)>(TEXT("FText::operator=(FText&&)"))
        .Pattern("48 8B 02 4C 8B 01 48 89 01 4C 89 02 48 8B 42 08 4C 8B 41 08 48 89 41 08 4C 89 42 08 8B 42 10 89 41 10 48 8B C1 C3")
        .Get();

    if (MoveAssign)
        return MoveAssign(this, Source);

    FMemory::Memswap(TextData, Source.TextData, HasRefCountedTextData() ? sizeof(ITextData*) : sizeof(TextData));
    GetFlags() = Source.GetFlags();

    return *this;
}

bool FText::IsEmpty() const
{
    static auto IsEmpty = Anchor::Function<bool (*)(const FText*)>(TEXT("FText::IsEmpty"))
        .Pattern("40 53 48 83 EC 20 48 8B D9 48 8B 09 48 8B 01 FF 50 38 48 8B C8 E8 ? ? ? ? 48 8B 0B 48 8B 01 FF 50 10 83 78 08 01 0F 9E C0")
        .Or()
        .Pattern("48 83 EC 28 E8 ? ? ? ? 83 78 08 01 0F 9E C0 48 83 C4 28 C3")
        .Or()
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
        .Or()
        .Pattern("83 EC 28 E8 ? ? ? ? 83 78 08 01 0F 9E C0 ? 83 ? ? C3")
        .Start()
        .Or()
        .Pattern("83 C4 D8 E8 ? ? ? ? 83 78 08 01 0F 9E C0 ? 83 ? ? C3")
        .Start()
        .Optional()
        .Get();

    if (IsEmpty)
        return IsEmpty(this);

    static auto ToString = Anchor::Function<const FString& (*)(const FText*)>(TEXT("FText::ToString"))
        .Pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B 0B 48 8B 01 48 83 C4 20 5B 48 FF 60 10")
        .Get();

    return ToString(this).IsEmpty();
}

FText FText::Format(FTextFormat Fmt, FFormatOrderedArguments&& InArguments)
{
    static auto Format = Anchor::Optional<FText (*)(FTextFormat, FFormatOrderedArguments&&)>(TEXT("FText::Format"))
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
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Find("mov r8, rax; lea rdx, *; lea rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Find("* r8, *; lea rdx, *; lea rcx, *; call 0x*")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; call 0x*; mov r8d, *; lea rdx, *; mov rcx, rax; call 0x*")
        .Find("lea rdx, [0x*]")
        .Find("mov r8, rax; lea rdx, *; lea rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Find("mov r8, rax; lea rdx, *; lea rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Find("lea rdx, [rbp-0x*]; lea rcx, [rsp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Find("lea rdx, [rsp+0x*]; lea rcx, [rbp-0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; mov r8b, 0x1; lea rdx, [0x*]")
        .Find("mov r8, *; lea rcx, *; mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Is("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x30; mov rbx, rdx; mov rdi, rcx; call 0x*")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rsp+0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; mov r8b, 0x1; lea rdx, [0x*]")
        .Find("mov r8, *; lea rcx, *; mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Is("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x30; mov rbx, rdx; mov rdi, rcx; call 0x*")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; mov rcx, *; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; mov r8b, 0x1; lea rdx, [0x*]")
        .Find("mov r8, *; lea rcx, *; mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Is("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x30; mov rbx, rdx; mov rdi, rcx; call 0x*")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea r8, *; lea rdx, *; lea rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Is("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x30; mov rbx, rdx; mov rdi, rcx; call 0x*")
        .Or()
        .Pattern("44 0F B6 80 ? ? 00 00 E8 ? ? ? ? 48 8D 15 ? ? ? ? 4C 8B E0 48 8D 4D ? E8")
        .Find("lea r8, [rsp+0x*]; lea rdx, [rbp-0x*]; lea rcx, [rbp-0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rsp+0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; call 0x*; mov r8d, *; lea rdx, *; mov rcx, rax; call 0x*")
        .Find("lea rdx, [0x*]")
        .Find("mov r8, rax; lea rdx, *; lea rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!Format)
        return FText();

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
    static auto Constructor = Anchor::Function<void (*)(FTextFormat*, const FText&, uint8)>(TEXT("FTextFormat::FTextFormat"))
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
        .Or()
        .Pattern("48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 50 48 8B DA 48 8B F9 33 F6 33 D2 B9 B0 00 00 00")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rsp+0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; mov r8b, 0x1; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; mov r8b, 0x1; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; mov rcx, *; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; mov r8b, 0x1; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Call()
        .Or()
        .Pattern("44 0F B6 80 ? ? 00 00 E8 ? ? ? ? 48 8D 15 ? ? ? ? 4C 8B E0 48 8D 4D ? E8")
        .Find("lea rdx, [0x*]")
        .Call()
        .Get();

    Constructor(this, InText, 1);
}

FFormatArgumentValue::FFormatArgumentValue(const FText& Value)
{
    static auto Constructor = Anchor::Optional<void (*)(FFormatArgumentValue*, const FText&)>(TEXT("FFormatArgumentValue::FFormatArgumentValue"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .TryCall("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Last("call 0x*")
        .Target()
        .Has("mov dword ptr [rcx], 0x4")
        .Get();

    if (Constructor)
    {
        Constructor(this, Value);
        return;
    }

    Type = 4;
    new (&TextValue) FText(Value);
    *(bool*)((uint8*)&TextValue + FText::GetTypeSize()) = true;
}
