#pragma once

#include "Containers/Array.h"

class UScriptStruct;

struct alignas(8) FUniqueNetIdRepl
{
    FUniqueNetIdRepl();

    FUniqueNetIdRepl(const FUniqueNetIdRepl&) = delete;

    ~FUniqueNetIdRepl();

    FUniqueNetIdRepl& operator=(const FUniqueNetIdRepl& Other);

    static UScriptStruct* StaticStruct();

private:
    uint8 Data[0x30];
};
