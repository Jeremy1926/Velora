#pragma once

#include "Misc/CoreMiscDefines.h"
#include "Containers/Array.h"
#include "Templates/SharedPointer.h"
#include "Templates/TypeCompatibleBytes.h"

class ITextData;
class FTextFormat;
class FFormatArgumentValue;

typedef TArray<FFormatArgumentValue> FFormatOrderedArguments;

class FText
{
public:
    FText();

    explicit FText(ENoInit)
    {
    }

    FText(const FText& Source);

    FText& operator=(const FText& Source);

    FText& operator=(FText&& Source);

    bool IsEmpty() const;

    static FText Format(FTextFormat Fmt, FFormatOrderedArguments&& InArguments);

    static FText Format(FTextFormat Fmt, FFormatArgumentValue v1);

private:
    TSharedRef<ITextData, ESPMode::ThreadSafe> TextData;
    uint32 Flags;
};

class FTextFormat
{
public:
    FTextFormat(const FText& InText);

private:
    TAlignedBytes<sizeof(TSharedRef<ITextData, ESPMode::ThreadSafe>), alignof(void*)> TextFormatData;
};

class FFormatArgumentValue
{
public:
    FFormatArgumentValue(const FText& Value);

private:
    int32 Type;

    union
    {
        int64 IntValue;
        uint64 UIntValue;
        float FloatValue;
        double DoubleValue;
    };

    TAlignedBytes<sizeof(FText), alignof(FText)> TextValue;
    bool bIsTextValueSet;
};
