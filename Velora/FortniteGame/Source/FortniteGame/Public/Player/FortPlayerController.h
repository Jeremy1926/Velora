#pragma once

#include "GameFramework/PlayerController.h"
#include "Building/BuildingActor.h"
#include "Items/FortInventory.h"
#include "Internationalization/Text.h"
#include "Misc/Guid.h"
#include "UObject/AssetPtr.h"

class ABuildingSMActor;
class AFortInventory;
class AFortPlayerPawn;
class AFortQuickBars;
class UFortItem;
class UFortItemDefinition;
class UFortMontageItemDefinitionBase;
class UFortRegisteredPlayerInfo;
class UStatManager;
enum class EFortQuickBars : uint8;
enum class EFortResourceType : uint8;

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

class AFortPlayerController : public APlayerController
{
public:
    static UClass* StaticClass();

    void ServerCreateBuildingActor_Implementation(FBuildingClassData BuildingClassData, FVector BuildLoc, FRotator BuildRot, bool bMirrored);

    void ServerBeginEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit);

    void ServerEditBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, int32 RotationIterations, bool bMirrored);

    void ServerEndEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToStopEditing);

    void ServerRepairBuildingActor_Implementation(ABuildingSMActor* BuildingActorToRepair);

    bool ValidateBuildingClassData(FBuildingClassData& BuildingClassData) const;

    bool CanPerformNativeAction(const FGameplayTag& ActionTag) const;

    bool IsBuildingRestricted(const FBuildingClassData& BuildingClassData, const FVector& BuildLoc, const FRotator& BuildRot) const;

    bool HasRequiredBuildingLevel(const FBuildingClassData& BuildingClassData) const;

    bool HasRequiredBuildingLevel(int32 BuildingLevel, EFortResourceType ResourceType) const;

    bool CanAffordToPlaceBuildableClass(FBuildingClassData BuildingClassData) const;

    EFortStructuralGridQueryResults CanPlaceBuildableClassInStructuralGrid(UClass* BuildingClass, FVector& BuildLoc, FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings);

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

    TAssetPtr<UObject>& GetStartRepairSound();

    void ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int32 PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot);

    class FOnMcpProfilesInitialized;

    bool InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized, bool bForceSync);

    void TryAddToQuickBar(UFortItem* Item);

    bool CanServerTryAddToQuickBar() const;

    void ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot);

    void ResetQuickBarToPrimary();

    UFortItem* K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const;

    bool RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars);

    void RemoveAllInventoryItems();

    void RemoveItemFromQuickBars(UFortItemDefinition* ItemDefinition);

    void ServerExecuteInventoryItem_Implementation(FGuid ItemGuid);

    void ServerSpawnInventoryDrop_Implementation(FGuid ItemGuid, int32 Count);

    void ServerRemoveInventoryStateValue_Implementation(FGuid ItemGuid, EFortItemEntryState::Type StateValueType);

    void ServerPlayEmoteItem_Implementation(UFortMontageItemDefinitionBase* EmoteAsset);

    void DropItemsOnPawnDestruction(AActor* DamageCauser, const FGameplayTagContainer& DeathTags, AFortPlayerPawn* DestroyedPawn, bool& bDroppedItems);

    AFortQuickBars*& GetQuickBars();

    AFortInventory*& GetWorldInventory();

    UFortRegisteredPlayerInfo*& GetMyPlayerInfo();

    UStatManager* GetStatManager() const;
};
