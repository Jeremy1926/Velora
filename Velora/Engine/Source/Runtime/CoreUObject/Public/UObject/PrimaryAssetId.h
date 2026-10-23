#pragma once

#include "UObject/NameTypes.h"

struct FPrimaryAssetType
{
    FName Name;
};

struct FPrimaryAssetId
{
    struct FOutlineNumberLayout
    {
        NAME_INDEX PrimaryAssetType;
        NAME_INDEX PrimaryAssetName;
    };

    static FPrimaryAssetId ParseTypeAndName(const FString& TypeAndName);

    FOutlineNumberLayout ToOutlineNumberLayout() const
    {
        FOutlineNumberLayout Layout{};
        TGameLayout<FName>::Store(&Layout.PrimaryAssetType, PrimaryAssetType.Name);
        TGameLayout<FName>::Store(&Layout.PrimaryAssetName, PrimaryAssetName);

        return Layout;
    }

    FPrimaryAssetType PrimaryAssetType;
    FName PrimaryAssetName;
};

template<>
struct TGameLayout<FPrimaryAssetId>
{
    using FStorage = FPrimaryAssetId;

    static FORCEINLINE FPrimaryAssetId Load(const void* Game)
    {
        return FPrimaryAssetId{ { TGameLayout<FName>::Load(Game) }, TGameLayout<FName>::Load((const uint8*)Game + TGameLayout<FName>::Size()) };
    }

    static FORCEINLINE void Store(void* Game, const FPrimaryAssetId& Value)
    {
        TGameLayout<FName>::Store(Game, Value.PrimaryAssetType.Name);
        TGameLayout<FName>::Store((uint8*)Game + TGameLayout<FName>::Size(), Value.PrimaryAssetName);
    }
};
