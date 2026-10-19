#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

class AFortPickup;
class AFortPlayerController;
struct FFortItemEntry;
class UFortWorldItemDefinition;
class USoundBase;

class UFortKismetLibrary : public UBlueprintFunctionLibrary
{
public:
    static void K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive);

    static int32 K2_RemoveItemFromPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval);

    static void BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier);

    static bool PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier);

    static AFortPickup* K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss);
};
