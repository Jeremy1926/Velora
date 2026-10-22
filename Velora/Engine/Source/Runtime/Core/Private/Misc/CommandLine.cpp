#include "pch.h"
#include "Misc/CommandLine.h"
#include "Scanner.h"

const TCHAR* FCommandLine::Get()
{
    static auto Get = Anchor::Function<const TCHAR* (*)()>(TEXT("FCommandLine::Get"))
        .String(TEXT("NOCONSOLE"))
        .Last("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Attempting to get the command line but it hasn't been initialized yet."), "lea rax, [0x*]; lea r8, [0x*]; mov [rsp+0x20], rax; lea rcx, [0x*]; lea edx, [r9+0x21]")
        .Caller()
        .Get();

    return Get();
}
