#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Misc/Crc.h"
#include <cwctype>

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

    FORCEINLINE bool IsEmpty() const
    {
        return Data.Num() <= 1;
    }

    FORCEINLINE int32 Len() const
    {
        return Data.Num() ? Data.Num() - 1 : 0;
    }

    FORCEINLINE void AppendChars(const TCHAR* Array, int32 Count)
    {
        if (!Count)
            return;

        int32 Index = Data.Num();

        Data.AddUninitialized(Count + (Index ? 0 : 1));

        TCHAR* EndPtr = Data.GetData() + Index - (Index ? 1 : 0);

        memcpy(EndPtr, Array, Count * sizeof(TCHAR));

        *(EndPtr + Count) = 0;
    }

    FORCEINLINE FString& operator+=(const TCHAR* Str)
    {
        AppendChars(Str, (int32)wcslen(Str));

        return *this;
    }

    FORCEINLINE FString& operator+=(const FString& Str)
    {
        AppendChars(*Str, Str.Len());

        return *this;
    }

    FORCEINLINE friend bool operator==(const FString& Lhs, const FString& Rhs)
    {
        for (auto String1 = *Lhs, String2 = *Rhs; *String1 || *String2; String1++, String2++)
        {
            if (towupper(*String1) != towupper(*String2))
                return false;
        }

        return true;
    }

    FORCEINLINE friend uint32 GetTypeHash(const FString& S)
    {
        return FCrc::Strihash_DEPRECATED(*S);
    }
};
