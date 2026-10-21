#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"
#include "UObject/ScriptInterface.h"
#include "Templates/SubclassOf.h"

class AActor;
class AFortAIDirector;
class AFortPickup;
class AFortPlayerController;
struct FActiveGameplayEffectHandle;
struct FFortAbilitySetHandle;
struct FFortItemEntry;
struct FFrame;
struct FGameplayTagContainer;
struct FGuid;
struct FTimespan;
class FString;
class IAbilitySystemInterface;
class IFortInventoryOwnerInterface;
class UFortAbilitySet;
class UFortItem;
class UFortWeaponItemDefinition;
class UFortWorldItemDefinition;
class UGameplayEffect;
class UScriptStruct;
class USoundBase;
enum class EFortPickupSourceTypeFlag : uint8;
enum class EFortPickupSpawnSource : uint8;
enum class EFortRarity : uint8;

struct FSpawnItemVariantParams
{
    FSpawnItemVariantParams();

    FSpawnItemVariantParams(const FSpawnItemVariantParams&) = delete;

    ~FSpawnItemVariantParams();

    static UScriptStruct* StaticStruct();

    UFortWorldItemDefinition* GetWorldItemDefinition() const;

    int32 GetNumberToSpawn() const;

    const FVector& GetPosition() const;

    const FVector& GetDirection() const;

    int32 GetOverrideMaxStackCount() const;

    bool GetToss() const;

    bool GetRandomRotation() const;

    bool GetBlockedFromAutoPickup() const;

    int32 GetPickupInstigatorHandle() const;

    EFortPickupSourceTypeFlag GetSourceType() const;

    EFortPickupSpawnSource GetSource() const;

    AFortPlayerController* GetOptionalOwnerPC() const;

    bool GetPickupOnlyRelevantToOwner() const;

private:
    uint8 Data[0x100];
};

class UFortKismetLibrary : public UBlueprintFunctionLibrary
{
public:
    static FFortAbilitySetHandle EquipFortAbilitySet(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterfaceActor, UFortAbilitySet* AbilitySet, UObject* OverrideSourceObject);

    static void GiveItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer);

    void execGiveItemToInventoryOwner(FFrame& Stack, void* const Z_Param__Result);

    static void K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive);

    static int32 K2_RemoveItemFromPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval);

    static int32 K2_RemoveItemFromPlayerByGuid(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 AmountToRemove, bool bForceRemoval);

    static int32 K2_RemoveFortItemFromPlayer(AFortPlayerController* PlayerController, UFortItem* Item, int32 AmountToRemove, bool bForceRemoval);

    void execK2_RemoveFortItemFromPlayer(FFrame& Stack, void* const Z_Param__Result);

    static AFortPlayerController* GetFortPlayerControllerFromActor(AActor* Actor);

    static float GetServerWorldTimeSeconds(const UObject* WorldContextObject);

    static void BroadcastAbilityImpactAtLocation(UObject* WorldContextObject, USoundBase* InSound, UObject* Instigator, const FVector& Position, float Radius, FName Tag, const bool bSubtractLocalNoise);

    static void BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier);

    static bool PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier);

    static FActiveGameplayEffectHandle ApplyGlobalEnvironmentGameplayEffectToActor(AActor* EffectTargetActor, TSubclassOf<UGameplayEffect> GameplayEffect, int32 GameplayEffectLevel, const FGameplayTagContainer& AdditionalContextTags);

    static AFortAIDirector* GetAIDirector(const UObject* WorldContextObject);

    void execGetAIDirector(FFrame& Stack, void* const Z_Param__Result);

    static AFortPickup* K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss);

    static AFortPickup* K2_SpawnPickupInWorldWithClass(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, TSubclassOf<AFortPickup> PickupClass, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss);

    static TArray<AFortPickup*> K2_SpawnPickupInWorldWithLootTier(UObject* WorldContextObject, FName LootTierName, const FVector& Position, int32 OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source);

    static AFortPickup* SpawnItemVariantPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* WorldItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, int32 PickupInstigatorHandle, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner);

    void execSpawnItemVariantPickupInWorld(FFrame& Stack, void* const Z_Param__Result);

    static void CreateTossAmmoPickupForWeaponItemDefinitionAtLocation(UObject* WorldContextObject, const UFortWeaponItemDefinition* WeaponItemDefinition, const FGameplayTagContainer& SourceTags, const FVector& Location, const EFortPickupSourceTypeFlag SourceTypeFlag, const EFortPickupSpawnSource SpawnSource);

    static bool AreAthenaVehiclesEnabled();

    static bool UpgradeAllWeaponsVerticalToRarity(AFortPlayerController* PlayerController, EFortRarity NewRarity);

    void execUpgradeAllWeaponsVerticalToRarity(FFrame& Stack, void* const Z_Param__Result);

    static void GetCalendarEventInformation(UObject* WorldContextObject, const FString& EventName, bool& bIsEventActive, FTimespan& TimeSinceBegin, FTimespan& TimeUntilEnd, float& TimespanRatio);
};
