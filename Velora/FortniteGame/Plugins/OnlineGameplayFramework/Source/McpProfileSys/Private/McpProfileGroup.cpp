#include "pch.h"
#include "Scanner.h"
#include "Detour.h"

PATCH(UMcpProfileGroup, SendRequestNow, Anchor::Patch()
    .String(TEXT("FATAL: Should not be possible to issue dedicated server commands without WITH_SERVER_CODE"))
    .Start()
    .Find("test *, *; jz 0x*; sub *, 0x1; jz 0x*")
    .Optional()
    .Find("jz 0x*", 2)
    .BranchToLast("jz 0x*"))

PATCH(UMcpProfileGroup, SendRequestNow, Anchor::Patch()
    .String(TEXT("FATAL: Should not be possible to issue dedicated server commands without WITH_SERVER_CODE"))
    .Start()
    .CallSite()
    .Is("call 0x*; lea rdx, [*]; jmp 0x*")
    .Optional()
    .Last("sub *, 0x1; jz 0x*; sub *, 0x1; jz 0x*")
    .Find("jz 0x*")
    .BranchToNext("jz 0x*", 2))
