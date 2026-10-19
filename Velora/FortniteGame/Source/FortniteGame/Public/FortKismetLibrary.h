#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"
#include "UObject/ScriptInterface.h"

class AFortPickup;
class AFortPlayerController;
struct FFortAbilitySetHandle;
struct FFortItemEntry;
struct FGuid;
class IAbilitySystemInterface;
class IFortInventoryOwnerInterface;
class UFortAbilitySet;
class UFortWorldItemDefinition;
class USoundBase;

class UFortKismetLibrary : public UBlueprintFunctionLibrary
{
public:
    static FFortAbilitySetHandle EquipFortAbilitySet(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterfaceActor, UFortAbilitySet* AbilitySet, UObject* OverrideSourceObject);

    static void GiveItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer);

    static void K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive);

    static int32 K2_RemoveItemFromPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval);

    static int32 K2_RemoveItemFromPlayerByGuid(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 AmountToRemove, bool bForceRemoval);

    static void BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier);

    static bool PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier);

    static AFortPickup* K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss);
};
