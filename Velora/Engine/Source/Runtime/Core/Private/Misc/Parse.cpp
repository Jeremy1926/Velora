#include "pch.h"
#include "Misc/Parse.h"
#include "Scanner.h"

bool FParse::Param(const TCHAR* Stream, const TCHAR* Param)
{
    static auto ParseParam = (bool (*)(const TCHAR*, const TCHAR*))Scanner::String(TEXT("NOCONSOLE")).Call().Get();

    return ParseParam(Stream, Param);
}
