#pragma once

#include "HAL/Platform.h"

typedef int32 NAME_INDEX;

enum EFindName
{
    FNAME_Find,
    FNAME_Add,
    FNAME_Replace_Not_Safe_For_Threading,
};

class FName
{
public:
    FName(const WIDECHAR* Name, EFindName FindType = FNAME_Add);

private:
    NAME_INDEX ComparisonIndex;
    int32 Number;
};
