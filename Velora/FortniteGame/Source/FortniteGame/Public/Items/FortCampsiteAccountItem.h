#pragma once

#include "Items/FortAccountItem.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Math/Vector.h"

class UScriptStruct;
struct FFortItemEntry;

struct FCachedItemData
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    const FString& GetItemDefinitionPersistentName() const;

    int32 GetItemCount() const;

    int32 GetItemLevel() const;

    int32 GetItemLoadedAmmo() const;
};

struct FCachedCampSpawnLocation
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FVector GetWorldSpawnLocation() const;
};

struct FCampsiteAccountData
{
    static UScriptStruct* StaticStruct();

    TRuntimeSizedArray<FCachedItemData>& GetStashedItems();

    TRuntimeSizedArray<FCachedCampSpawnLocation>& GetCampSpawnLocations();

    void CopyFromMCPData(TArray<FFortItemEntry>& ItemEntriesOut);
};

class UFortCampsiteAccountItem : public UFortAccountItem
{
public:
    static UClass* StaticClass();

    FCampsiteAccountData& GetCampsiteAccountData();
};
