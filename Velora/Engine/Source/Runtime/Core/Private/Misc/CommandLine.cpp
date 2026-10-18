#include "pch.h"
#include "Misc/CommandLine.h"
#include "Scanner.h"

const TCHAR* FCommandLine::Get()
{
    static auto Get = (const TCHAR* (*)())Scanner::String(TEXT("NOCONSOLE")).Last("call 0x*").Target();

    return Get();
}
