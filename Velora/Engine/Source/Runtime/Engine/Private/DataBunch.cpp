#include "pch.h"
#include "Net/DataBunch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"

void* FOutBunch::operator new(size_t Size)
{
    static auto New = Anchor::Function<void* (*)(size_t)>(TEXT("operator new"))
        .String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s"))
        .Start()
        .Find("mov ecx, 0x*; call 0x*; test rax, rax")
        .Call()
        .Get();

    static auto BunchSize = (size_t)Scanner::String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s")).Start().Find("mov ecx, 0x*; call 0x*; test rax, rax").Imm();

    return New(BunchSize);
}

void FOutBunch::operator delete(void* Ptr)
{
    FMemory::Free(Ptr);
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

void FOutBunch::SetReliable(bool bInReliable)
{
    static auto Close = Scanner::String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")).Start().Find("mov r8b, 0x1");
    static auto Base = Close.Find("lea rcx, [rsp+0x*]").Disp();
    static auto Reliable = Close.Find("mov byte ptr [rsp+0x*], 0x1");
    static auto Bits = Close.Find("movzx *, byte ptr [rsp+0x*]");
    static auto Offset = Scanner::Offset(TEXT("FOutBunch::bReliable"), (Reliable.Get() ? Reliable.Disp() : Bits.Disp()) - Base);
    static auto Mask = Reliable.Get() ? (uint8_t)1 : (uint8_t)(Bits.Find("or cl, 0x*").Imm() << Bits.Find("shl cl, 0x*").Imm());

    auto& Flags = *((uint8_t*)this + Offset);

    Flags = (Flags & ~Mask) | (bInReliable ? Mask : 0);
}

void FOutBunch::SetDormant(bool bInDormant)
{
    static auto Close = Scanner::String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")).Start().Find("mov r8b, 0x1");
    static auto Base = Close.Find("lea rcx, [rsp+0x*]").Disp();
    static auto Reliable = Close.Find("mov byte ptr [rsp+0x*], 0x1");
    static auto Bits = Close.Find("movzx *, byte ptr [rsp+0x*]");
    static auto Offset = Reliable.Get() ? Scanner::Offset(TEXT("FOutBunch::bDormant"), Reliable.Find("mov [rsp+0x*], al").Disp() - Base) : Scanner::Offset(TEXT("FOutBunch::bReliable"), Bits.Disp() - Base);
    static auto Mask = Reliable.Get() ? (uint8_t)1 : (uint8_t)~Bits.Find("and al, 0x*").Imm();

    auto& Flags = *((uint8_t*)this + Offset);

    Flags = (Flags & ~Mask) | (bInDormant ? Mask : 0);
}
