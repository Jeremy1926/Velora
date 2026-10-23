#include "pch.h"
#include "Net/DataBunch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"

int32 FOutBunch::GetTypeSize()
{
    static auto BunchSize = (int32)Anchor::Offset(TEXT("sizeof(FOutBunch)"))
        .String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s"))
        .Start()
        .Find("mov ecx, 0x*; call 0x*; test rax, rax")
        .Or()
        .String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s"))
        .Start()
        .Find("mov ecx, 0x*; inc [*]; call 0x*; test rax, rax")
        .Value();

    return BunchSize;
}

void* FOutBunch::operator new(size_t Size)
{
    static auto New = Anchor::Function<void* (*)(size_t, uint32)>(TEXT("operator new"))
        .String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s"))
        .Start()
        .Find("mov ecx, 0x*; call 0x*; test rax, rax")
        .Call()
        .Or()
        .String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s"))
        .Start()
        .Find("mov ecx, 0x*; inc [*]; call 0x*; test rax, rax")
        .Call()
        .Get();

    return New(GetTypeSize(), 0);
}

void FOutBunch::operator delete(void* Ptr)
{
    FMemory::Free(Ptr);
}

FOutBunch::FOutBunch()
{
    static auto Constructor = Anchor::Function<FOutBunch* (*)(FOutBunch*)>(TEXT("FOutBunch::FOutBunch"))
        .Pattern("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 03 33 C0 89 83 ? ? ? ? 48 89 83 ? ? ? ? 48 89 83 ? ? ? ? 48 89 83 ? ? ? ? 48 89 83 ? ? ? ? 48 8B C3 48 83 C4 20 5B C3")
        .Get();

    Constructor(this);
}

FOutBunch::FOutBunch(UChannel* InChannel, bool bInClose)
{
    static auto Constructor = Anchor::Function<FOutBunch* (*)(FOutBunch*, UChannel*, bool)>(TEXT("FOutBunch::FOutBunch"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Get();

    Constructor(this, InChannel, bInClose);
}

FOutBunch::~FOutBunch()
{
    (**(void (***)(FOutBunch*, uint32))this)(this, 0);
}

int64 FOutBunch::GetNumBits() const
{
    static auto Num = Anchor::Offset(TEXT("FBitWriter::Num"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Call()
        .Call()
        .Find("mov [rdi+0x*], rax")
        .Disp();

    return *(const int64*)((const uint8_t*)this + Num);
}

void FOutBunch::SetNext(FOutBunch* InNext)
{
    static auto Next = Anchor::Offset(TEXT("FOutBunch::Next"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Find("mov [rsi+0x*], rbx; mov [rsi+0x*], rcx")
        .Find("mov [rsi+0x*], rcx")
        .Disp();

    *(FOutBunch**)((uint8_t*)this + Next) = InNext;
}

void FOutBunch::SetChannel(UChannel* InChannel)
{
    static auto Channel = Anchor::Offset(TEXT("FOutBunch::Channel"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Find("mov [rsi+0x*], rbx; mov [rsi+0x*], rcx")
        .Disp();

    *(UChannel**)((uint8_t*)this + Channel) = InChannel;
}

void FOutBunch::SetChIndex(int32 InChIndex)
{
    static auto ChIndex = Anchor::Offset(TEXT("FOutBunch::ChIndex"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Find("mov eax, [rbx+0x*]; mov [rsi+0x*], eax")
        .Find("mov [rsi+0x*], eax")
        .Disp();

    *(int32*)((uint8_t*)this + ChIndex) = InChIndex;
}

void FOutBunch::SetChName(const FName& InChName)
{
    static auto ChName = Anchor::Offset(TEXT("FOutBunch::ChName"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Find("mov eax, [rbx+0x*]; mov [rsi+0x*], eax")
        .Find("mov *, [rbx+0x*]")
        .Find("mov [rsi+0x*], *ax")
        .Disp();

    TGameLayout<FName>::Store((uint8_t*)this + ChName, InChName);
}

void FOutBunch::SetReliable(bool bInReliable)
{
    static auto bReliable = Anchor::Offset(TEXT("FOutBunch::bReliable"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Origin("lea rcx, [rsp+0x*]")
        .Find("mov byte ptr [rsp+0x*], 0x1")
        .Optional()
        .Disp();

    if (bReliable)
    {
        auto& Flags = *((uint8_t*)this + bReliable);

        Flags = (Flags & ~1) | (bInReliable ? 1 : 0);
        return;
    }

    static auto CloseFlags = Anchor::Offset(TEXT("FOutBunch::bClose"))
        .String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s"))
        .Start()
        .Find("test byte ptr [*+0x*], 0x4; jz *; cmp byte ptr [*+0x*], 0x1")
        .Or()
        .String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s"))
        .Start()
        .Find("test byte ptr [*+0x*], 0x4; jz *; cmp [*+0x*], *")
        .Optional()
        .Disp();

    static auto ReliableMask = (uint8_t)Anchor::Offset(TEXT("FOutBunch::bReliable"))
        .If(CloseFlags > 0)
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Find("or byte ptr [*+0x*], 0x*")
        .Optional()
        .Value();

    if (CloseFlags > 0 && ReliableMask)
    {
        auto& Flags = *((uint8_t*)this + CloseFlags);

        Flags = (Flags & ~ReliableMask) | (bInReliable ? ReliableMask : 0);
        return;
    }

    static auto Offset = Anchor::Offset(TEXT("FOutBunch::bReliable"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Origin("lea rcx, [rsp+0x*]")
        .Find("movzx *, byte ptr [rsp+0x*]")
        .Disp();

    static auto Bit = Anchor::Offset(TEXT("FOutBunch::bReliable"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Find("movzx *, byte ptr [rsp+0x*]")
        .Find("or cl, 0x*")
        .Value();

    static auto Shift = Anchor::Offset(TEXT("FOutBunch::bReliable"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Find("movzx *, byte ptr [rsp+0x*]")
        .Find("shl cl, 0x*")
        .Value();

    auto Mask = (uint8_t)(Bit << Shift);
    auto& Flags = *((uint8_t*)this + Offset);

    Flags = (Flags & ~Mask) | (bInReliable ? Mask : 0);
}

void FOutBunch::SetDormant(bool bInDormant)
{
    static auto bReliable = Anchor::Offset(TEXT("FOutBunch::bReliable"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Find("mov byte ptr [rsp+0x*], 0x1")
        .Optional()
        .Value();

    if (bReliable)
    {
        static auto bDormant = Anchor::Offset(TEXT("FOutBunch::bDormant"))
            .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
            .Start()
            .Find("mov r8b, 0x1")
            .Origin("lea rcx, [rsp+0x*]")
            .Find("mov byte ptr [rsp+0x*], 0x1")
            .Find("mov [rsp+0x*], al")
            .Disp();

        auto& Flags = *((uint8_t*)this + bDormant);

        Flags = (Flags & ~1) | (bInDormant ? 1 : 0);
        return;
    }

    static auto Offset = Anchor::Offset(TEXT("FOutBunch::bReliable"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Origin("lea rcx, [rsp+0x*]")
        .Find("movzx *, byte ptr [rsp+0x*]")
        .Optional()
        .Disp();

    static auto Mask = (uint8_t)Anchor::Offset(TEXT("FOutBunch::bDormant"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Find("movzx *, byte ptr [rsp+0x*]")
        .Find("and al, 0x*")
        .Optional()
        .Mask();

    if (Offset <= 0 || !Mask)
        return;

    auto& Flags = *((uint8_t*)this + Offset);

    Flags = (Flags & ~Mask) | (bInDormant ? Mask : 0);
}
