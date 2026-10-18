#include "pch.h"
#include "Misc/Guid.h"
#include "Scanner.h"

FGuid FGuid::NewGuid()
{
    static auto NewGuid = (FGuid* (*)(FGuid*))Scanner::String(TEXT("FIX GUID FOR: %s")).Call(3).Get();

    FGuid Result;
    NewGuid(&Result);

    return Result;
}
