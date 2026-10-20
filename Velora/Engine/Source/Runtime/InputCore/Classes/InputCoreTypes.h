#pragma once

#include "UObject/NameTypes.h"
#include "Templates/SharedPointer.h"

struct FKeyDetails;

struct FKey
{
    FKey(const FName InName)
        : KeyName(InName)
    {
    }

    bool operator==(const FKey& Other) const
    {
        return KeyName == Other.KeyName;
    }

    FName KeyName;
    mutable TSharedPtr<FKeyDetails> KeyDetails;
};
