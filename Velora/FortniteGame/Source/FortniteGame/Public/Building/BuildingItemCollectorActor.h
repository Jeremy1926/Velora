#pragma once

#include "Building/BuildingActor.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "UObject/NameTypes.h"

class AFortPlayerController;
class UFortWorldItemDefinition;
class UScriptStruct;
struct FFortItemEntry;
struct FScalableFloat;

struct FColletorUnitInfo
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    UFortWorldItemDefinition*& GetInputItem();

    const FScalableFloat& GetInputCount();

    bool GetUseDefinedOutputItem();

    UFortWorldItemDefinition*& GetOutputItem();

    TArray<FFortItemEntry>& GetOutputItemEntry();

    FName GetOverrideOutputItemLootTierGroupName();
};

template<>
class TArray<FColletorUnitInfo, FDefaultAllocator> : public TRuntimeSizedArray<FColletorUnitInfo>
{
};

class ABuildingItemCollectorActor : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    void AthenaPostApplyPlaylistDataTables();

    void CheckDepositGoalReached(AFortPlayerController* Player, UFortWorldItemDefinition* Item);

    void GrantOutput(UFortWorldItemDefinition* Item);

    static void TickPendingOutputs();

    FName GetDefaultItemLootTierGroupName() const;

    TArray<FColletorUnitInfo>& GetItemCollections();

    AFortPlayerController*& GetControllingPlayer();

    float GetDespensingDelayTime() const;

    FVector GetLootSpawnLocation() const;

    int32& GetStartingGoalLevel();
};
