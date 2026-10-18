#include "pch.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

const FEngineVersion& FEngineVersion::Current()
{
    static auto Current = (const FEngineVersion& (*)())Scanner::String(TEXT("Engine Version: %s")).Last("call 0x*").Last("call 0x*").Target();

    return Current();
}
