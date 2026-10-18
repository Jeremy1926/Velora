#pragma once

#include "HAL/Platform.h"

struct FParse
{
    static bool Param(const TCHAR* Stream, const TCHAR* Param);

    static bool Value(const TCHAR* Stream, const TCHAR* Match, int32& Value);
};
