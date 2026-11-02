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
        : TextData()
        , Flags(0)
    {
    }

    FText(const FText& Source);

    ~FText();

    FText& operator=(const FText& Source);

    FText& operator=(FText&& Source);

    bool IsEmpty() const;

    static int32 GetTypeSize();

    static FText Format(FTextFormat Fmt, FFormatOrderedArguments&& InArguments);

    static FText Format(FTextFormat Fmt, FFormatArgumentValue v1);

private:
    static bool HasRefCountedTextData();

    FORCEINLINE uint32& GetFlags()
    {
        return HasRefCountedTextData() ? *(uint32*)(TextData + sizeof(ITextData*)) : Flags;
    }

    alignas(void*) uint8 TextData[sizeof(TSharedRef<ITextData, ESPMode::ThreadSafe>)];
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
