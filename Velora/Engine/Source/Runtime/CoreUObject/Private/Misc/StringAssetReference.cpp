#include "pch.h"
#include "Misc/StringAssetReference.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"

FString FStringAssetReference::GetLongPackageName() const
{
    FString PackageName;

    if (auto Delimiter = wcschr(*AssetLongPathname, TEXT('.')))
        PackageName.AppendChars(*AssetLongPathname, (int32)(Delimiter - *AssetLongPathname));

    return PackageName;
}

UObject* FStringAssetReference::TryLoad() const
{
    UObject* LoadedObject = nullptr;

    if (!AssetLongPathname.IsEmpty())
        LoadedObject = LoadObject<UObject>(nullptr, *AssetLongPathname);

    return LoadedObject;
}
