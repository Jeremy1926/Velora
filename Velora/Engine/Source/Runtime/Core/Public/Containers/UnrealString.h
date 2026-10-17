#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"

class FString
{
private:
    typedef TArray<TCHAR> DataType;
    DataType Data;

public:
    FString() = default;
    FString(FString&&) = default;
    FString(const FString&) = default;

    FORCEINLINE FString(const TCHAR* Src)
    {
        if (Src && *Src)
        {
            int32 SrcLen = (int32)wcslen(Src) + 1;
            Data.AddUninitialized(SrcLen);

            memcpy(Data.GetData(), Src, SrcLen * sizeof(TCHAR));
        }
    }

    FORCEINLINE FString(const ANSICHAR* Src)
    {
        if (Src && *Src)
        {
            int32 SrcLen = (int32)strlen(Src) + 1;
            Data.AddUninitialized(SrcLen);

            for (int32 Index = 0; Index < SrcLen; Index++)
                Data.GetData()[Index] = (uint8)Src[Index] < 0x80 ? (TCHAR)Src[Index] : TEXT('?');
        }
    }

    FORCEINLINE const TCHAR* operator*() const
    {
        return Data.Num() ? Data.GetData() : TEXT("");
    }
};
