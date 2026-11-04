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
        .Or()
        .String(TEXT("Engine Version: %s"))
        .Start()
        .Find("call 0x*; lea r8d, [*]; mov rcx, rax")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetEngineVersion"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("call 0x*")
        .Target()
        .Get();

    return Current();
}
