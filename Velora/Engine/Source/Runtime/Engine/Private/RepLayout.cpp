#include "pch.h"
#include "Net/RepLayout.h"
#include "Scanner.h"

void FRepLayout::SendPropertiesForRPC(UFunction* Function, UActorChannel* Channel, FNetBitWriter& Writer, void* Data) const
{
    static auto SendPropertiesForRPC = Anchor::Function<void (*)(const FRepLayout*, UFunction*, UActorChannel*, FNetBitWriter&, void*)>(TEXT("FRepLayout::SendPropertiesForRPC"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("and dword ptr [rsp+0x*], 0x0")
        .Find("call 0x*")
        .Target()
        .Find("xor r8d, r8d; call 0x*; mov rcx, [rbp+0x*]; lea rdx, [rbp-0x*]; mov r8, r*; call 0x*")
        .Find("call 0x*", 3)
        .Target()
        .Get();

    SendPropertiesForRPC(this, Function, Channel, Writer, Data);
}

void FRepLayout::BuildSharedSerializationForRPC(void* Data)
{
    static auto BuildSharedSerializationForRPC = Anchor::Function<void (*)(FRepLayout*, void*)>(TEXT("FRepLayout::BuildSharedSerializationForRPC"))
        .String(TEXT("UNetDriver::ProcessRemoteFunction: No owning connection for actor %s. Function %s will not be processed."))
        .Start()
        .Find("call 0x*; mov r8, [rbp+0x*]; lea rax, [rbp-0x*]; mov [rsp+0x*], rax")
        .Target()
        .Get();

    BuildSharedSerializationForRPC(this, Data);
}

void FRepLayout::ClearSharedSerializationForRPC()
{
    static auto ClearSharedSerializationForRPC = Anchor::Function<void (*)(FRepLayout*)>(TEXT("FRepLayout::ClearSharedSerializationForRPC"))
        .String(TEXT("UNetDriver::ProcessRemoteFunction: No owning connection for actor %s. Function %s will not be processed."))
        .Start()
        .Find("call 0x*; mov r8, [rbp+0x*]; lea rax, [rbp-0x*]; mov [rsp+0x*], rax")
        .Find("call 0x*", 2)
        .Target()
        .Get();

    ClearSharedSerializationForRPC(this);
}
