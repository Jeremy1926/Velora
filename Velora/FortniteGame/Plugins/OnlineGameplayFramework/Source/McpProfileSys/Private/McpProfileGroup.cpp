#include "pch.h"
#include "Scanner.h"
#include "Detour.h"

static auto SendRequestNow = []
{
    auto Fatal = Scanner::String(TEXT("FATAL: Should not be possible to issue dedicated server commands without WITH_SERVER_CODE"));
    auto Server = Fatal.Last(std::format("jz 0x{:x}", (uintptr_t)Fatal.Get()));

    return PatchBranch(TEXT("UMcpProfileGroup::SendRequestNow"), Server.Get(), Server.Last("jz 0x*").Target());
}();
