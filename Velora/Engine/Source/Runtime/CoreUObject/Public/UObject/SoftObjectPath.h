#pragma once

#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

class UObject;

struct FSoftObjectPath
{
public:
    FORCEINLINE bool IsValid() const
    {
        return AssetPathName != NAME_None;
    }

    FString ToString() const;

    FString GetLongPackageName() const;

    UObject* TryLoad() const;

    FORCEINLINE void Reset()
    {
        AssetPathName = NAME_None;
        SubPathString = FString();
    }

    static FSoftObjectPath GetOrCreateIDForObject(const UObject* Object);

    static int32 GetCurrentTag();

private:
    FName AssetPathName;
    FString SubPathString;
};
