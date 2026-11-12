#include "pch.h"
#include "Misc/Guid.h"
#include "Scanner.h"

FGuid FGuid::NewGuid()
{
    static auto CreateGuid = Anchor::Data<long (**)(FGuid*)>(TEXT("FPlatformMisc::CreateGuid"))
        .String(TEXT("FIX GUID FOR: %s"))
        .Find("lea rcx, [rsp+0x*]; mov [rsp+0x*], *; mov [rsp+0x*], *; call [0x*]")
        .Find("call [0x*]")
        .Target()
        .Optional()
        .Get();

    if (CreateGuid)
    {
        FGuid Result;
        (*CreateGuid)(&Result);

        return Result;
    }

    static auto NewGuid = Anchor::Function<FGuid* (*)(FGuid*)>(TEXT("FGuid::NewGuid"))
        .String(TEXT("FIX GUID FOR: %s"))
        .Call(3)
        .Get();

    FGuid Result;
    NewGuid(&Result);

    return Result;
}
