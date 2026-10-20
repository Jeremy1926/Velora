#include "pch.h"
#include "Misc/CommandLine.h"
#include "Scanner.h"

const TCHAR* FCommandLine::Get()
{
    static auto Get = Anchor::Function<const TCHAR* (*)()>(TEXT("FCommandLine::Get"))
        .String(TEXT("NOCONSOLE"))
        .Last("call 0x*")
        .Target()
        .Get();

    return Get();
}
