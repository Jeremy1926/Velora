#include "pch.h"
#include "Net/DataBunch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"

struct FCloseBunch
{
    const uint8_t* Constructor;
    int32_t Flags;
    uint8_t DormantMask;
    uint8_t ReliableMask;
};

static auto CloseBunch = []
{
    auto Close = Scanner::String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s")).Start().Find("mov r8b, 0x1");
    auto Flags = Close.Find("movzx *, byte ptr [rsp+0x*]");
    FCloseBunch Bunch;

    Bunch.Constructor = Close.Call().Get();
    Bunch.Flags = Flags.Disp() - Close.Find("lea rcx, [rsp+0x*]").Disp();
    Bunch.DormantMask = (uint8_t)~Flags.Find("and al, 0x*").Imm();
    Bunch.ReliableMask = (uint8_t)(Flags.Find("or cl, 0x*").Imm() << Flags.Find("shl cl, 0x*").Imm());

    return Bunch;
}();

void* FOutBunch::operator new(size_t Size)
{
    static auto Allocation = Scanner::String(TEXT("SendBunch: Reliable partial bunch overflows reliable buffer! %s")).Start().Find("mov ecx, 0x*; call 0x*; test rax, rax");
    static auto New = (void* (*)(size_t))Allocation.Call().Get();
    static auto BunchSize = (size_t)Allocation.Imm();

    return New(BunchSize);
}

void FOutBunch::operator delete(void* Ptr)
{
    FMemory::Free(Ptr);
}

FOutBunch::FOutBunch(UChannel* InChannel, bool bInClose)
{
    ((FOutBunch* (*)(FOutBunch*, UChannel*, bool))CloseBunch.Constructor)(this, InChannel, bInClose);
}

FOutBunch::~FOutBunch()
{
    (**(void (***)(FOutBunch*, uint32))this)(this, 0);
}

void FOutBunch::SetReliable(bool bInReliable)
{
    auto& Flags = *((uint8_t*)this + CloseBunch.Flags);

    Flags = (Flags & ~CloseBunch.ReliableMask) | (bInReliable ? CloseBunch.ReliableMask : 0);
}

void FOutBunch::SetDormant(bool bInDormant)
{
    auto& Flags = *((uint8_t*)this + CloseBunch.Flags);

    Flags = (Flags & ~CloseBunch.DormantMask) | (bInDormant ? CloseBunch.DormantMask : 0);
}
