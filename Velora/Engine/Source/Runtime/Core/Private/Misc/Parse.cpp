#include "pch.h"
#include "Misc/Parse.h"
#include "Scanner.h"

bool FParse::Param(const TCHAR* Stream, const TCHAR* Param)
{
    static auto ParseParam = Anchor::Function<bool (*)(const TCHAR*, const TCHAR*)>(TEXT("FParse::Param"))
        .String(TEXT("NOCONSOLE"))
        .Call()
        .Or()
        .String(TEXT("Multiprocess"))
        .Find("call 0x*")
        .Target()
        .Get();

    return ParseParam(Stream, Param);
}

bool FParse::Value(const TCHAR* Stream, const TCHAR* Match, int32& Value)
{
    static auto ParseValue = Anchor::Function<bool (*)(const TCHAR*, const TCHAR*, int32&)>(TEXT("FParse::Value"))
        .String(TEXT("MaxTeamSize="))
        .Find("call 0x*")
        .Target()
        .Get();

    return ParseValue(Stream, Match, Value);
}
