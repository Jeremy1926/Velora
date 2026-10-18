#include "pch.h"
#include "Misc/Parse.h"
#include "Scanner.h"

bool FParse::Param(const TCHAR* Stream, const TCHAR* Param)
{
    static auto ParseParam = (bool (*)(const TCHAR*, const TCHAR*))Scanner::String(TEXT("NOCONSOLE")).Call().Get(TEXT("FParse::Param"));

    return ParseParam(Stream, Param);
}

bool FParse::Value(const TCHAR* Stream, const TCHAR* Match, int32& Value)
{
    static auto ParseValue = (bool (*)(const TCHAR*, const TCHAR*, int32&))Scanner::String(TEXT("MaxTeamSize=")).Find("call 0x*").Target(TEXT("FParse::Value"));

    return ParseValue(Stream, Match, Value);
}
