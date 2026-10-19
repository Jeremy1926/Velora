#include "pch.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"

FString FSoftObjectPath::ToString() const
{
    if (!IsValid())
        return FString();

    FString FullPathString = AssetPathName.ToString();

    if (!SubPathString.IsEmpty())
    {
        FullPathString += TEXT(":");
        FullPathString += SubPathString;
    }

    return FullPathString;
}

FString FSoftObjectPath::GetLongPackageName() const
{
    FString PathName = AssetPathName.ToString();
    FString PackageName;

    if (auto Delimiter = wcschr(*PathName, TEXT('.')))
        PackageName.AppendChars(*PathName, (int32)(Delimiter - *PathName));

    return PackageName;
}

UObject* FSoftObjectPath::TryLoad() const
{
    UObject* LoadedObject = nullptr;

    if (IsValid())
        LoadedObject = LoadObject<UObject>(nullptr, *ToString());

    return LoadedObject;
}
