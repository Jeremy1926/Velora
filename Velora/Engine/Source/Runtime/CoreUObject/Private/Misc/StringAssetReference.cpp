#include "pch.h"
#include "Misc/StringAssetReference.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"

UObject* FStringAssetReference::TryLoad() const
{
    UObject* LoadedObject = nullptr;

    if (!AssetLongPathname.IsEmpty())
        LoadedObject = LoadObject<UObject>(nullptr, *AssetLongPathname);

    return LoadedObject;
}
