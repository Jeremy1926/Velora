#pragma once

#include "HAL/Platform.h"
#include "UObject/UnrealNames.h"

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
    FORCEINLINE bool operator==(const FName& Other) const
    {
        return CompositeComparisonValue == Other.CompositeComparisonValue;
    }

    FORCEINLINE bool operator!=(const FName& Other) const
    {
        return !(*this == Other);
    }

    friend FORCEINLINE uint32 GetTypeHash(const FName N)
    {
        return N.ComparisonIndex + N.Number;
    }

    FORCEINLINE FName(EName N)
        : ComparisonIndex(N)
        , Number(0)
    {
    }

    FName(const WIDECHAR* Name, EFindName FindType = FNAME_Add);

private:
    union
    {
        struct
        {
            NAME_INDEX ComparisonIndex;
            uint32 Number;
        };

        uint64 CompositeComparisonValue;
    };
};
