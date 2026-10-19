#include "pch.h"
#include "Scanner.h"
#include "Detour.h"

static auto SendRequestNow = []
{
    auto Context = Scanner::String(TEXT("FATAL: Should not be possible to issue dedicated server commands without WITH_SERVER_CODE")).Start().Find("test ecx, ecx; jz 0x*; sub ecx, 0x1; jz 0x*");

    return PatchBranch(TEXT("UMcpProfileGroup::SendRequestNow"), Context.Find("jz 0x*", 2).Get(), Context.Find("jz 0x*").Target());
}();
