#include "pch.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

const FEngineVersion& FEngineVersion::Current()
{
    static auto Current = Anchor::Unnamed<const FEngineVersion& (*)()>()
        .String(TEXT("Engine Version: %s"))
        .Last("call 0x*")
        .Last("call 0x*")
        .Target()
        .Get();

    return Current();
}
