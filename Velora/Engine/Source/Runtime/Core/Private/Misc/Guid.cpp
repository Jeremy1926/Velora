#include "pch.h"
#include "Misc/Guid.h"
#include "Scanner.h"

FGuid FGuid::NewGuid()
{
    static auto NewGuid = Scanner::Function<FGuid* (*)(FGuid*)>(Scanner::String(TEXT("FIX GUID FOR: %s")).Call(3).Get(TEXT("FGuid::NewGuid")));

    FGuid Result;
    NewGuid(&Result);

    return Result;
}
