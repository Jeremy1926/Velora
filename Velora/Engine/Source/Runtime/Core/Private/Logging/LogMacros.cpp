#include "pch.h"
#include "Logging/LogMacros.h"
#include "Scanner.h"

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

decltype(FMsg::Logf_Internal) FMsg::Logf_Internal = Anchor::Function<decltype(FMsg::Logf_Internal)>(TEXT("FMsg::Logf_Internal"))
    .String(TEXT("AddClientConnection: Added client connection: %s"))
    .Call()
    .Is("sub rsp, *; mov rax, [r9]; lea r8, [0x*]")
    .Find("call 0x*")
    .Target()
    .Or()
    .String(TEXT("AddClientConnection: Added client connection: %s"))
    .Call()
    .Is("mov [rsp+0x10], rdx; mov [rsp+0x18], r8; mov [rsp+0x20], r9")
    .String(TEXT("Script Msg: %s"))
    .Find("call 0x*")
    .Target()
    .Or()
    .String(TEXT("AddClientConnection: Added client connection: %s"))
    .Call()
    .Is("mov rax, rsp; mov [rax+0x10], rdx; mov [rax+0x18], r8; mov [rax+0x20], r9")
    .String(TEXT("Script Msg: %s"))
    .Find("call 0x*")
    .Target()
    .Or()
    .String(TEXT("AddClientConnection: Added client connection: %s"))
    .Call()
    .Get();
