#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"

template<>
void TArray<TCHAR>::ResizeGrow(int32 OldNum);

class FString
{
private:
    typedef TArray<TCHAR> DataType;
    DataType Data;

public:
    FString() = default;
    FString(FString&&) = default;

    FORCEINLINE FString(const TCHAR* Src)
    {
        if (Src && *Src)
        {
            int32 SrcLen = (int32)wcslen(Src) + 1;
            Data.AddUninitialized(SrcLen);

            memcpy(Data.GetData(), Src, SrcLen * sizeof(TCHAR));
        }
    }

    FORCEINLINE const TCHAR* operator*() const
    {
        return Data.Num() ? Data.GetData() : TEXT("");
    }
};
