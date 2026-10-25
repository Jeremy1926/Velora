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
