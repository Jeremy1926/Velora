#pragma once

#include "HAL/Platform.h"
#include "Containers/UnrealString.h"
#include "Templates/UnrealTemplate.h"
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
        return IsOutlineNumber() ? ComparisonIndex == Other.ComparisonIndex : CompositeComparisonValue == Other.CompositeComparisonValue;
    }

    FORCEINLINE bool operator!=(const FName& Other) const
    {
        return !(*this == Other);
    }

    friend uint32 GetTypeHash(const FName N);

    FName(EName N);

    FName(const WIDECHAR* Name, EFindName FindType = FNAME_Add);

    FString ToString() const;

    static bool IsOutlineNumber();

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

template<>
struct TGameLayout<FName>
{
    using FStorage = FName;

    static FORCEINLINE int32 Size()
    {
        return FName::IsOutlineNumber() ? (int32)sizeof(NAME_INDEX) : (int32)sizeof(FName);
    }

    static FORCEINLINE FName Load(const void* Game)
    {
        FName Value(NAME_None);
        memcpy(&Value, Game, Size());

        return Value;
    }

    static FORCEINLINE void Store(void* Game, const FName& Value)
    {
        memcpy(Game, &Value, Size());
    }
};
