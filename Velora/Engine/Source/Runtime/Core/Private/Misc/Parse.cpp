#include "pch.h"
#include <cwctype>
#include "Misc/Parse.h"
#include "Scanner.h"

bool FParse::Param(const TCHAR* Stream, const TCHAR* Param)
{
    static auto ParseParam = Anchor::Function<bool (*)(const TCHAR*, const TCHAR*)>(TEXT("FParse::Param"))
        .String(TEXT("NOCONSOLE"))
        .Call()
        .Or()
        .Pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 33 ED 48 8B F2 48 8B F9 66 39 29 74 ? 41 B0 01 E8")
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

bool FParse::Command(const TCHAR** Stream, const TCHAR* Match, bool bParseMightTriggerExecution)
{
    while (**Stream == TEXT(' ') || **Stream == 9)
        (*Stream)++;

    const int32 MatchLen = (int32)wcslen(Match);

    if (_wcsnicmp(*Stream, Match, MatchLen) == 0 && !iswalnum((*Stream)[MatchLen]))
    {
        *Stream += MatchLen;

        while (**Stream == TEXT(' ') || **Stream == 9)
            (*Stream)++;

        return true;
    }

    return false;
}
