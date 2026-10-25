#pragma once

#include "GameFramework/PlayerController.h"
#include "Building/BuildingActor.h"
#include "Items/FortInventory.h"
#include "Internationalization/Text.h"
#include "Misc/Guid.h"
#include "UObject/AssetPtr.h"
#include "Containers/Map.h"
#include "Delegates/Delegate.h"
#include "UObject/NameTypes.h"

class ABuildingSMActor;
class AFortPlayerController;
class AFortInventory;
class AFortPawn;
class AFortPlayerPawn;
class AFortQuickBars;
class UFortItem;
class UFortItemDefinition;
class UFortMcpProfile;
class UFortMontageItemDefinitionBase;
class UFortRegisteredPlayerInfo;
class UFortWorldItem;
class UStatManager;
struct FFrame;
struct FTransform;
enum class EFortQuickBars : uint8;
enum class EFortResourceType : uint8;

enum class ESubGame : uint8
{
    Campaign,
    Athena,
    Invalid,
};

enum class EFortStructuralGridQueryResults : uint8
{
    CanAdd,
    ExistingActor,
    Obstructed,
    NoStructuralSupport,
    InvalidActor,
    ReachedLimit,
    NoEditPermission,
    PatternNotPermittedByLayoutRequirement,
    ResourceTypeNotPermittedByLayoutRequirement,
    BuildingAtRequirementsDisabled,
    BuildingOtherThanRequirementsDisabled,
    EFortStructuralGridQueryResults_MAX
};

enum class EFortBuildPreviewMarkerOptionalAdjustment : uint8
{
    None,
    FreeWallPieceOnTop,
    FreeWallPieceOnBottom,
};

struct FFortPlayerControllerEvents
{
    static TMulticastDelegate<void, AFortPlayerController*, const FName&, const FName&>& GetOnPlayerStateChangedEvent();
};

class AFortPlayerController : public APlayerController
{
public:
    static UClass* StaticClass();

    void ServerCreateBuildingActor_Implementation(FBuildingClassData BuildingClassData, const TGameValue<FVector>& GameBuildLoc, const TGameValue<FRotator>& GameBuildRot, bool bMirrored);

    void ServerCreateBuildingActor_Implementation(const FCreateBuildingActorData& CreateBuildingData);

    ABuildingSMActor* ServerCreateBuildingActorInternal(FBuildingClassData BuildingClassData, const TGameValue<FVector>& GameBuildLoc, const TGameValue<FRotator>& GameBuildRot, bool bMirrored, float SyncKey, bool bForceOverrideInteractBuildCheck, bool bIgnoreCostCheck, bool bIgnoreBuildingValidityCheck);

    void ServerBeginEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit);

    void ServerEditBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, uint8 RotationIterations, bool bMirrored);

    void ServerEndEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToStopEditing);

    void ServerRepairBuildingActor_Implementation(ABuildingSMActor* BuildingActorToRepair);

    bool ValidateBuildingClassData(FBuildingClassData& BuildingClassData) const;

    bool CanPerformNativeAction(const FGameplayTag& ActionTag) const;

    bool IsBuildingRestricted(const FBuildingClassData& BuildingClassData, const FVector& BuildLoc, const FRotator& BuildRot) const;

    bool HasRequiredBuildingLevel(const FBuildingClassData& BuildingClassData) const;

    bool HasRequiredBuildingLevel(int32 BuildingLevel, EFortResourceType ResourceType) const;

    bool CanAffordToPlaceBuildableClass(FBuildingClassData BuildingClassData) const;

    EFortStructuralGridQueryResults CanPlaceBuildableClassInStructuralGrid(UClass* BuildingClass, FVector& BuildLoc, FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings, EFortBuildPreviewMarkerOptionalAdjustment& MarkerOptionalAdjustment);

    int32 PayBuildableClassPlacementCost(FBuildingClassData BuildingClassData);

    FText GetErrorMessageFromStructuralQuery(EFortStructuralGridQueryResults QueryResults) const;

    bool HasPermissionToEditWorld(ABuildingActor* BuildingActor, const FVector& Location, const FRotator& Rotation) const;

    bool CanAffordToRepair(ABuildingSMActor* BuildingActorToRepair) const;

    int32 PayBuildingRepairCost(ABuildingSMActor* BuildingActorToRepair);

    int32 GetWorldPlayerId() const;

    void ClientSendMessage(const FText& MESSAGE);

    void ClientExecuteInventoryItem(const FGuid& ItemGuid, float Delay, bool bForceExecute, bool bActivateSlotAfterSettingFocused);

    void TriggerUIFeedbackEvent(FName EventName);

    AFortPlayerPawn* GetMyFortPawn() const;

    UFortMcpProfile* GetAthenaProfile() const;

    FMulticastScriptDelegate& GetOnMcpProfilesInitializedEvent();

    bool IsPlayerAlive();

    TAssetPtr<UObject>& GetStartRepairSound();

    void ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int32 PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot);

    class FOnMcpProfilesInitialized;

    bool InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized, bool bForceSync);

    void TryAddToQuickBar(UFortItem* Item);

    bool CanServerTryAddToQuickBar() const;

    void ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot);

    void ClientActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot, bool bForceExecution);

    void ResetQuickBarToPrimary();

    bool ForceEquipValidWeapon();

    UFortItem* K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const;

    bool RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars);

    void RemoveAllInventoryItems();

    void DropAllItems(const UFortItemDefinition* IgnoreItemDef, const UFortItemDefinition* AdditionalIgnoreItemDef, bool bIgnoreBuildingMaterials);

    void execDropAllItems(FFrame& Stack, void* const Z_Param__Result);

    void DropSpecificItem(const UFortItemDefinition* DropItemDef);

    void execDropSpecificItem(FFrame& Stack, void* const Z_Param__Result);

    void RemoveItemFromQuickBars(UFortItemDefinition* ItemDefinition);

    void ServerExecuteInventoryItem_Implementation(FGuid ItemGuid);

    void ServerSpawnInventoryDrop_Implementation(FGuid ItemGuid, int32 Count);

    void ServerRemoveInventoryStateValue_Implementation(FGuid ItemGuid, EFortItemEntryState::Type StateValueType);

    void ServerPlayEmoteItem_Implementation(UFortMontageItemDefinitionBase* EmoteAsset);

    void ServerCheat_Implementation(const FString& Msg);

    void Cheat(const FString& Msg);

    AActor* SpawnToyInstance(UClass* ToyClass, const FTransform& SpawnPosition);

    void execSpawnToyInstance(FFrame& Stack, void* const Z_Param__Result);

    TArray<AActor*>& GetActiveToyInstances();

    TMap<UClass*, int32>& GetToySummonCounts();

    void DropItemsOnPawnDestruction(AActor* DamageCauser, const FGameplayTagContainer& DeathTags, AFortPlayerPawn* DestroyedPawn, bool& bDroppedItems);

    static void DropItemsAsPickups(const TArray<UFortWorldItem*>& Items, AFortPawn* Pawn, AFortInventory* Inventory, AFortPlayerController* PlayerController);

    AFortQuickBars* GetQuickBars();

    void SetQuickBars(AFortQuickBars* InQuickBars);

    AFortInventory*& GetWorldInventory();

    UFortRegisteredPlayerInfo*& GetMyPlayerInfo();

    UStatManager* GetStatManager() const;

    void SetAirSpawnControlRotation(const FRotator& InAirSpawnControlRotation);

    AFortInventory* GetViewTargetInventory() const;

    void SetInfiniteAmmo(bool bInInfiniteAmmo);

    void SetBuildFree(bool bInBuildFree);
};

inline const TCHAR* const GCheatCommands[][2] =
{
    { TEXT("Help"), TEXT("Lists every cheat command with what it does") },
    { TEXT("StartAircraft"), TEXT("Starts the battle bus right now, same as Start Bus in the GUI") },
    { TEXT("God"), TEXT("Toggles god mode, you take no damage (AFortPawn::bIsInvulnerable)") },
};
