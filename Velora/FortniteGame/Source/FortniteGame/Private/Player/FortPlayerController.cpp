#include "pch.h"
#include "Player/FortPlayerController.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Player/FortPlayerState.h"
#include "Player/FortPlayerStateZone.h"
#include "Building/BuildingSMActor.h"
#include "Items/FortInventory.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortQuickBars.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Items/FortResourceItemDefinition.h"
#include "Pawns/FortPlayerPawn.h"
#include "Weapons/FortWeap_BuildingTool.h"
#include "Weapons/FortWeap_EditingTool.h"
#include "AI/FortAIFunctionLibrary.h"
#include "Analytics/FortAnalytics.h"
#include "FortGameData.h"
#include "FortGameMode.h"
#include "FortGameStateZone.h"
#include "FortKismetLibrary.h"
#include "FortniteGame.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Math/TransformVectorized.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

static auto InitializeMcpProfile = Detour(TEXT("AFortPlayerController::InitializeMcpProfile"), Scanner::String(TEXT("Initialized MCP profile for %s*")).Start().Get(), &AFortPlayerController::InitializeMcpProfile);

static auto RemoveAllInventoryItems = []
{
    auto StartAircraftPhase = Scanner::String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase")).Find("mov byte ptr [rsp+0x20], 0x1");
    auto Call = StartAircraftPhase.Find("call 0x*", 3);

    return Call.Target() == StartAircraftPhase.Find("call 0x*").Target() ? DetourCall(TEXT("AFortPlayerController::RemoveAllInventoryItems"), Call.Get(), &AFortPlayerController::RemoveAllInventoryItems) : nullptr;
}();

static auto ActivateSlot = DetourCall(TEXT("AFortPlayerController::ActivateSlot"), Scanner::String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase")).Find("mov byte ptr [rsp+0x20], 0x1").Find("call 0x*").Get(), &AFortPlayerController::ActivateSlot);

static auto DropItemsOnPawnDestruction = []
{
    auto Deaths = Scanner::String(TEXT("Deaths")).Start();
    auto Call = Deaths.Find("mov byte ptr [rsp+0x*], 0x0; mov rcx, *; call 0x*");

    if (Call.Get())
        return DetourCall(TEXT("AFortPlayerController::DropItemsOnPawnDestruction"), Call.Find("call 0x*").Get(), &AFortPlayerController::DropItemsOnPawnDestruction);

    auto Slot = Deaths.Find("mov byte ptr [rsp+0x*], 0x0; call [rax+0x*]").Find("call [rax+0x*]").Slot(TEXT("AFortPlayerController::DropItemsOnPawnDestruction"));
    auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    return DetourVTable(TEXT("AFortPlayerController::DropItemsOnPawnDestruction"), (void**)(VTable + Slot), &AFortPlayerController::DropItemsOnPawnDestruction);
}();

static Scanner::FCode ClientBuild()
{
    static auto ClientBuild = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerCreateBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();

    return ClientBuild;
}

static Scanner::FCode ClientRepair()
{
    static auto ClientRepair = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerRepairBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();

    return ClientRepair;
}

static int32_t IsBuildingRestrictedSlot()
{
    static auto IsBuildingRestricted = ClientBuild().Find("or *, 0xffffffff").Find("call [*+0x*]").Slot(TEXT("AFortPlayerController::IsBuildingRestricted"));

    return IsBuildingRestricted;
}

static int32_t PlayerControllerSlot(const void* Function)
{
    auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
    auto Index = 0;

    while (VTable[Index] != Function)
        Index++;

    return Index * (int32_t)sizeof(void*);
}

template<typename FunctionType>
static void InstallImplementation(const TCHAR* Name, const TCHAR* FunctionPath, FunctionType Implementation)
{
    auto Slot = Scanner::Exec(FunctionPath).Find("call [rax+*]", 2).Slot(Name);
    auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    DetourVTable(Name, (void**)(VTable + Slot), Implementation);
}

bool AFortPlayerController::InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized, bool bForceSync)
{
    static auto Building = []
    {
        InstallImplementation(TEXT("AFortPlayerController::ServerCreateBuildingActor_Implementation"), TEXT("/Script/FortniteGame.FortPlayerController:ServerCreateBuildingActor"), &AFortPlayerController::ServerCreateBuildingActor_Implementation);
        InstallImplementation(TEXT("AFortPlayerController::ServerBeginEditingBuildingActor_Implementation"), TEXT("/Script/FortniteGame.FortPlayerController:ServerBeginEditingBuildingActor"), &AFortPlayerController::ServerBeginEditingBuildingActor_Implementation);
        InstallImplementation(TEXT("AFortPlayerController::ServerEditBuildingActor_Implementation"), TEXT("/Script/FortniteGame.FortPlayerController:ServerEditBuildingActor"), &AFortPlayerController::ServerEditBuildingActor_Implementation);
        InstallImplementation(TEXT("AFortPlayerController::ServerEndEditingBuildingActor_Implementation"), TEXT("/Script/FortniteGame.FortPlayerController:ServerEndEditingBuildingActor"), &AFortPlayerController::ServerEndEditingBuildingActor_Implementation);
        InstallImplementation(TEXT("AFortPlayerController::ServerRepairBuildingActor_Implementation"), TEXT("/Script/FortniteGame.FortPlayerController:ServerRepairBuildingActor"), &AFortPlayerController::ServerRepairBuildingActor_Implementation);

        return true;
    }();

    static auto ServerExecuteInventoryItem = []
    {
        auto ServerExecuteInventoryItem_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerExecuteInventoryItem")).Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerController::ServerExecuteInventoryItem_Implementation"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

        return DetourVTable(TEXT("AFortPlayerController::ServerExecuteInventoryItem_Implementation"), (void**)(VTable + ServerExecuteInventoryItem_Implementation), &AFortPlayerController::ServerExecuteInventoryItem_Implementation);
    }();

    static auto ServerSpawnInventoryDrop = []
    {
        auto ServerSpawnInventoryDrop_Implementation = Scanner::Exec(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ServerSpawnInventoryDrop")) ? TEXT("/Script/FortniteGame.FortPlayerController:ServerSpawnInventoryDrop") : TEXT("/Script/FortniteGame.FortPlayerController:ServerAttemptInventoryDrop")).Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerController::ServerSpawnInventoryDrop_Implementation"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

        return DetourVTable(TEXT("AFortPlayerController::ServerSpawnInventoryDrop_Implementation"), (void**)(VTable + ServerSpawnInventoryDrop_Implementation), &AFortPlayerController::ServerSpawnInventoryDrop_Implementation);
    }();

    static auto ServerRemoveInventoryStateValue = []
    {
        InstallImplementation(TEXT("AFortPlayerController::ServerRemoveInventoryStateValue_Implementation"), TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryStateValue"), &AFortPlayerController::ServerRemoveInventoryStateValue_Implementation);

        return true;
    }();

    static auto ServerRemoveItemInternal = []
    {
        auto ServerRemoveItemInternal_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortQuickBars:ServerRemoveItemInternal")).Find("call [*+0x*]", 2).Slot(TEXT("AFortQuickBars::ServerRemoveItemInternal_Implementation"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortQuickBars"));

        return DetourVTable(TEXT("AFortQuickBars::ServerRemoveItemInternal_Implementation"), (void**)(VTable + ServerRemoveItemInternal_Implementation), &AFortQuickBars::ServerRemoveItemInternal_Implementation);
    }();

    static auto RemoveInventoryItem = []
    {
        auto PlayerController = (uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
        auto ServerRemoveInventoryItem_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem")).Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerController::ServerRemoveInventoryItem_Implementation"));
        auto RemoveInventoryItem = Scanner::At(*(void**)(*(uint8_t**)PlayerController + ServerRemoveInventoryItem_Implementation)).Find("call [*+0x*]");
        static auto InventoryOwnerOffset = RemoveInventoryItem.Last("add rcx, 0x*").Imm(TEXT("AFortPlayerController::IFortInventoryOwnerInterface"));

        auto InventoryOwnerRemoveInventoryItem = +[](uint8_t* InventoryOwnerInterface, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars)
        {
            return ((AFortPlayerController*)(InventoryOwnerInterface - InventoryOwnerOffset))->RemoveInventoryItem(ItemGuid, Count, bForceRemoveFromQuickBars);
        };

        return DetourVTable(TEXT("AFortPlayerController::RemoveInventoryItem"), (void**)(*(uint8_t**)(PlayerController + InventoryOwnerOffset) + RemoveInventoryItem.Slot(TEXT("IFortInventoryOwnerInterface::RemoveInventoryItem"))), (void*)InventoryOwnerRemoveInventoryItem);
    }();

    static auto SetLoadedAmmo = []
    {
        auto PlayerController = (uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
        auto ServerRemoveInventoryItem_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem")).Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerController::ServerRemoveInventoryItem_Implementation"));
        auto InventoryOwner = Scanner::At(*(void**)(*(uint8_t**)PlayerController + ServerRemoveInventoryItem_Implementation)).Find("call [*+0x*]").Last("add rcx, 0x*").Imm(TEXT("AFortPlayerController::IFortInventoryOwnerInterface"));
        auto InventoryOwnerVTable = *(const uint8_t***)(PlayerController + InventoryOwner);
        auto Index = 0;

        while (!Scanner::At(InventoryOwnerVTable[Index]).Start().Has("jmp [r8+*]"))
            Index++;

        auto SetLoadedAmmo = Scanner::At(InventoryOwnerVTable[Index]).Start().Find("jmp [r8+*]").Slot(TEXT("UFortWorldItem::SetLoadedAmmo"));
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWorldItem"));

        return DetourVTable(TEXT("UFortWorldItem::SetLoadedAmmo"), (void**)(VTable + SetLoadedAmmo), &UFortWorldItem::SetLoadedAmmo);
    }();

    auto bInitialized = (this->*::InitializeMcpProfile)(OnMcpProfilesInitialized, bForceSync);

    if (HasAuthority() && !GetQuickBars())
    {
        FActorSpawnParameters SpawnInfo;
        SpawnInfo.Owner = this;

        GetQuickBars() = (AFortQuickBars*)GetWorld()->SpawnActor(AFortQuickBars::StaticClass(), nullptr, SpawnInfo);
    }

    return bInitialized;
}

void AFortPlayerController::AddItemToQuickBars(UFortItemDefinition* ItemDefinition, EFortQuickBars QuickBarType, int32 SlotIndex)
{
    static auto AddItemToQuickBars = Scanner::Function<void (*)(AFortPlayerController*, UFortItemDefinition*, EFortQuickBars, int32)>(Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerController:AddItemToQuickBars")).Get(TEXT("AFortPlayerController::AddItemToQuickBars")));

    AddItemToQuickBars(this, ItemDefinition, QuickBarType, SlotIndex);
}

static Scanner::FCode TryAddToQuickBarFunction()
{
    static auto TryAddToQuickBar = Scanner::String(TEXT("Failed TryAddToQuickBar due to invalid ItemDefinition for Item '%s'")).Start();

    return TryAddToQuickBar;
}

void AFortPlayerController::TryAddToQuickBar(UFortItem* Item)
{
    static auto TryAddToQuickBar = []
    {
        auto Function = TryAddToQuickBarFunction().Get(TEXT("AFortPlayerController::TryAddToQuickBar"));
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
        auto Index = 0;

        while (VTable[Index] != Function)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortPlayerController*, UFortItem*))(VTable + TryAddToQuickBar))(this, Item);
}

bool AFortPlayerController::CanServerTryAddToQuickBar() const
{
    static auto bCanServerTryAddToQuickBar = !TryAddToQuickBarFunction().Has("cmp byte ptr [rcx+0x*], 0x3; mov rdi, rdx");

    return bCanServerTryAddToQuickBar;
}

void AFortPlayerController::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    if (GetQuickBars())
        GetQuickBars()->ActivateSlot(InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot);

    if (!::RemoveAllInventoryItems)
        RemoveAllInventoryItems();
}

UFortItem* AFortPlayerController::K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const
{
    static auto K2_FindExistingItemForDefinition = Scanner::Function<UFortItem* (*)(const AFortPlayerController*, const UFortItemDefinition*, bool)>(Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerController:K2_FindExistingItemForDefinition")).Get(TEXT("AFortPlayerController::K2_FindExistingItemForDefinition")));

    return K2_FindExistingItemForDefinition(this, ItemDefinition, bInStorageVault);
}

void AFortPlayerController::ServerExecuteInventoryItem_Implementation(FGuid ItemGuid)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    for (auto Item : WorldInventory->GetInventory().ItemInstances)
    {
        if (Item->GetItemGuid() == ItemGuid)
        {
            if (auto ItemDefinition = Cast<UFortWorldItemDefinition>(Item->GetItemDefinitionBP()))
                ItemDefinition->ServerExecute(Item, this);

            return;
        }
    }
}

void AFortPlayerController::ServerSpawnInventoryDrop_Implementation(FGuid ItemGuid, int32 Count)
{
    auto WorldInventory = GetWorldInventory();
    auto Pawn = GetMyFortPawn();

    if (!WorldInventory || !Pawn || Count <= 0)
        return;

    for (auto& ItemEntry : WorldInventory->GetInventory().ReplicatedEntries)
    {
        if (ItemEntry.GetItemGuid() != ItemGuid)
            continue;

        auto ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition());

        if (!ItemDefinition || !ItemDefinition->CanBeDropped())
            return;

        FFortItemEntry DropItemEntry(ItemEntry.GetItemDefinition(), FMath::Min(Count, ItemEntry.GetCount()), ItemEntry.GetLevel());
        DropItemEntry.GetLoadedAmmo() = ItemEntry.GetLoadedAmmo();
        DropItemEntry.GetDurability() = ItemEntry.GetDurability();

        const FVector Location = Pawn->GetActorLocation();

        if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), DropItemEntry, Location, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), nullptr, EFortPickupSourceTypeFlag::Player))
            Pickup->TossPickup(Location, Pawn, 0, true);

        RemoveInventoryItem(ItemGuid, DropItemEntry.GetCount(), false);

        return;
    }
}

void AFortPlayerController::ServerRemoveInventoryStateValue_Implementation(FGuid ItemGuid, EFortItemEntryState::Type StateValueType)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    auto& Inventory = WorldInventory->GetInventory();

    for (auto& ItemEntry : Inventory.ReplicatedEntries)
    {
        if (ItemEntry.GetItemGuid() != ItemGuid)
            continue;

        ItemEntry.RemoveStateValue(StateValueType);
        Inventory.MarkItemDirty(ItemEntry);
        WorldInventory->HandleInventoryLocalUpdate();

        return;
    }
}

void AFortPlayerController::DropItemsOnPawnDestruction(AActor* DamageCauser, const FGameplayTagContainer& DeathTags, AFortPlayerPawn* DestroyedPawn, bool& bDroppedItems)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory || !DestroyedPawn)
        return;

    auto& ReplicatedEntries = WorldInventory->GetInventory().ReplicatedEntries;
    const FVector Location = DestroyedPawn->GetActorLocation();

    for (int32 Index = ReplicatedEntries.Num() - 1; Index >= 0; Index--)
    {
        auto ItemDefinition = Cast<UFortWorldItemDefinition>(ReplicatedEntries[Index].GetItemDefinition());

        if (!ItemDefinition || !ItemDefinition->ShouldDropOnDeath() || ReplicatedEntries[Index].GetCount() <= 0)
            continue;

        FFortItemEntry DropItemEntry(ItemDefinition, ReplicatedEntries[Index].GetCount(), ReplicatedEntries[Index].GetLevel());
        DropItemEntry.GetLoadedAmmo() = ReplicatedEntries[Index].GetLoadedAmmo();
        DropItemEntry.GetDurability() = ReplicatedEntries[Index].GetDurability();

        const FGuid ItemGuid = ReplicatedEntries[Index].GetItemGuid();

        if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), DropItemEntry, Location, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), nullptr, EFortPickupSourceTypeFlag::Player))
            Pickup->TossPickup(Location, DestroyedPawn, 0, true);

        RemoveInventoryItem(ItemGuid, DropItemEntry.GetCount(), true);

        bDroppedItems = true;
    }
}

UClass* AFortPlayerController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController"));

    return Class;
}

void AFortPlayerController::ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int32 PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot)
{
    static auto ClientReportDamagedResourceBuilding = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ClientReportDamagedResourceBuilding"));

    struct
    {
        ABuildingSMActor* BuildingSMActor;
        EFortResourceType PotentialResourceType;
        int32 PotentialResourceCount;
        bool bDestroyed;
        bool bJustHitWeakspot;
    } Parms{ BuildingSMActor, PotentialResourceType, PotentialResourceCount, bDestroyed, bJustHitWeakspot };

    CallRemoteFunction(ClientReportDamagedResourceBuilding, &Parms, nullptr, nullptr);
}

bool AFortPlayerController::RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return false;

    auto& Inventory = WorldInventory->GetInventory();

    for (int32 Index = 0; Index < Inventory.ReplicatedEntries.Num(); Index++)
    {
        auto& ItemEntry = Inventory.ReplicatedEntries[Index];

        if (ItemEntry.GetItemGuid() != ItemGuid)
            continue;

        auto ItemDefinition = ItemEntry.GetItemDefinition();

        if (ItemEntry.GetCount() > Count)
        {
            ItemEntry.SetCount(ItemEntry.GetCount() - Count);
            Inventory.MarkItemDirty(ItemEntry);

            if (bForceRemoveFromQuickBars)
                RemoveItemFromQuickBars(ItemDefinition);
        }
        else
        {
            RemoveItemFromQuickBars(ItemDefinition);

            Inventory.ReplicatedEntries.RemoveAt(Index);
            Inventory.MarkArrayDirty();
            WorldInventory->ForceNetUpdate();
        }

        WorldInventory->HandleInventoryLocalUpdate();

        return true;
    }

    return false;
}

void AFortPlayerController::RemoveAllInventoryItems()
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode());
    auto PlayerController = Cast<AFortPlayerControllerAthena>(this);
    auto CustomizationLoadout = PlayerController ? PlayerController->GetCustomizationLoadout() : nullptr;
    auto Pickaxe = CustomizationLoadout ? CustomizationLoadout->GetPickaxe() : nullptr;
    auto& ReplicatedEntries = WorldInventory->GetInventory().ReplicatedEntries;

    for (int32 Index = ReplicatedEntries.Num() - 1; Index >= 0; Index--)
    {
        auto ItemDefinition = ReplicatedEntries[Index].GetItemDefinition();
        auto bKeep = Pickaxe && (UFortItemDefinition*)Pickaxe->GetWeaponDefinition() == ItemDefinition;

        if (GameMode)
        {
            for (auto& StartingItem : GameMode->GetStartingItems())
                bKeep |= StartingItem.Item == ItemDefinition && StartingItem.Count > 0;
        }

        if (bKeep)
            continue;

        const FGuid ItemGuid = ReplicatedEntries[Index].GetItemGuid();

        RemoveInventoryItem(ItemGuid, ReplicatedEntries[Index].GetCount(), true);
    }
}

void AFortPlayerController::RemoveItemFromQuickBars(UFortItemDefinition* ItemDefinition)
{
    static auto RemoveItemFromQuickBars = Scanner::Function<void (*)(AFortPlayerController*, UFortItemDefinition*)>(Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars")).Get(TEXT("AFortPlayerController::RemoveItemFromQuickBars")));

    RemoveItemFromQuickBars(this, ItemDefinition);
}

AFortQuickBars*& AFortPlayerController::GetQuickBars()
{
    static auto QuickBars = GetClass()->FindPropertyByName(FName(TEXT("QuickBars")));

    return *QuickBars->ContainerPtrToValuePtr<AFortQuickBars*>(this);
}

AFortInventory*& AFortPlayerController::GetWorldInventory()
{
    static auto WorldInventory = GetClass()->FindPropertyByName(FName(TEXT("WorldInventory")));

    return *WorldInventory->ContainerPtrToValuePtr<AFortInventory*>(this);
}

UFortRegisteredPlayerInfo*& AFortPlayerController::GetMyPlayerInfo()
{
    static auto MyPlayerInfo = GetClass()->FindPropertyByName(FName(TEXT("MyPlayerInfo")));

    return *MyPlayerInfo->ContainerPtrToValuePtr<UFortRegisteredPlayerInfo*>(this);
}

void AFortPlayerController::ServerCreateBuildingActor_Implementation(FBuildingClassData BuildingClassData, FVector BuildLoc, FRotator BuildRot, bool bMirrored)
{
    if (!ValidateBuildingClassData(BuildingClassData) || !BuildingClassData.BuildingClass)
        return;

    auto BuildingClass = BuildingClassData.BuildingClass;

    if (!BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return;

    if (!CanPerformNativeAction(FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.InteractBuild")))))
        return;

    const bool bBuildingRestricted = IsBuildingRestricted(BuildingClassData, BuildLoc, BuildRot);
    TArray<ABuildingActor*> ExistingBuildings;
    bool bCanAfford = false;
    auto QueryResults = EFortStructuralGridQueryResults::CanAdd;

    if (!bBuildingRestricted)
    {
        bCanAfford = CanAffordToPlaceBuildableClass(BuildingClassData);

        const bool bHasRequiredBuildingLevel = HasRequiredBuildingLevel(BuildingClassData);
        const FVector OldBuildLoc = BuildLoc;
        const FRotator OldBuildRot = BuildRot;

        QueryResults = CanPlaceBuildableClassInStructuralGrid(BuildingClass, BuildLoc, BuildRot, bMirrored, &ExistingBuildings);

        if (!BuildLoc.Equals(OldBuildLoc) || !BuildRot.Equals(OldBuildRot))
        {
            UE_LOG(LogFort, Warning, TEXT("ServerCreateBuildingActor_Implementation() yielded different build location after calling CanPlaceBuildableClassInStructuralGrid() - X=%3.3f Y=%3.3f Z=%3.3f P=%f Y=%f R=%f  to  X=%3.3f Y=%3.3f Z=%3.3f P=%f Y=%f R=%f"), OldBuildLoc.X, OldBuildLoc.Y, OldBuildLoc.Z, OldBuildRot.Pitch, OldBuildRot.Yaw, OldBuildRot.Roll, BuildLoc.X, BuildLoc.Y, BuildLoc.Z, BuildRot.Pitch, BuildRot.Yaw, BuildRot.Roll);
        }

        if (bCanAfford && bHasRequiredBuildingLevel && QueryResults == EFortStructuralGridQueryResults::CanAdd)
        {
            for (auto ExistingBuilding : ExistingBuildings)
            {
                if (ExistingBuilding && ExistingBuilding->GetDestroyOnPlayerBuildingPlacement())
                    ExistingBuilding->Die(FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Effect.InstantDeath.StructuralSupport"))), nullptr, nullptr);
            }

            FActorSpawnParameters SpawnParameters;
            SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            auto BuildingActor = (ABuildingSMActor*)GetWorld()->SpawnActor(BuildingClass->IsChildOf(ABuildingActor::StaticClass()) ? BuildingClass : nullptr, &BuildLoc, &BuildRot, SpawnParameters);

            if (!BuildingActor)
                return;

            BuildingActor->SetCurrentBuildingLevel(BuildingClassData.UpgradeLevel);
            BuildingActor->SetMirrored(bMirrored);
            BuildingActor->InitializeBuildingActor(EFortBuildingInitializationReason::Spawned, GetWorldPlayerId(), nullptr, nullptr, false);
            BuildingActor->PostInitializeSpawnedBuildingActor(EFortBuildingInitializationReason::Spawned);

            UFortAnalytics::FireEvent_BuildingAction(this, TEXT("Create"), BuildingActor, PayBuildableClassPlacementCost(BuildingClassData));

            if (auto GameMode = Cast<AFortGameMode>(GetWorld()->GetAuthGameMode()))
                GameMode->OnPlayerCreatedBuilding(this, BuildingActor);

            if (auto MyFortPawn = GetMyFortPawn())
            {
                UFortAIFunctionLibrary::MakeNoiseEventAtLocation(MyFortPawn, 1000.0f, BuildingActor->GetActorLocation());

                MyFortPawn->OnBlueprintPlace();

                if (auto BuildingTool = Cast<AFortWeap_BuildingTool>(MyFortPawn->GetCurrentWeapon()))
                    BuildingTool->PlayWeaponFireFX(false);
            }

            return;
        }
    }

    static auto BuildingIsRestrictedText = (const FText*)ClientBuild().Find("lea rdx, [0x*]; lea rcx, *; call 0x*").Target(TEXT("BuildingIsRestrictedText"));
    static auto UnableToAfford = (const FText*)ClientBuild().Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*").Find("lea rdx, [0x*]").Target(TEXT("UnableToAfford"));

    FText Message;

    if (bBuildingRestricted)
        Message = *BuildingIsRestrictedText;
    else
        Message = GetErrorMessageFromStructuralQuery(QueryResults);

    if (Message.IsEmpty())
    {
        auto BuildingSMActor = (ABuildingSMActor*)BuildingClass->GetDefaultObject();

        Message = FText::Format(FTextFormat(*UnableToAfford), FFormatArgumentValue(UFortGameData::Get()->GetBuildingResourceName(BuildingSMActor->GetResourceType())));
    }

    if (!Message.IsEmpty())
        ClientSendMessage(Message);

    TriggerUIFeedbackEvent(!bBuildingRestricted && !bCanAfford ? FName(TEXT("BuildPreviewUnableToAfford")) : FName(TEXT("BuildPreviewUnableToPlace")));
}

void AFortPlayerController::ServerBeginEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit)
{
    if (!BuildingActorToEdit || !GetMyFortPawn() || !BuildingActorToEdit->CanBePlayerEdited(this))
        return;

    auto PlayerState = GetPlayerState();

    if (!PlayerState)
        return;

    BuildingActorToEdit->SetEditingPlayer(Cast<AFortPlayerStateZone>(PlayerState));

    auto EditToolItemDefinition = UFortGameData::Get()->GetEditToolItem();
    auto EditToolItem = K2_FindExistingItemForDefinition(EditToolItemDefinition, false);

    if (EditToolItem && EditToolItemDefinition && EditToolItemDefinition->ServerExecute((UFortWorldItem*)EditToolItem, this))
    {
        if (auto EditTool = Cast<AFortWeap_EditingTool>(GetMyFortPawn()->GetCurrentWeapon()))
            EditTool->SetEditActor(BuildingActorToEdit);
    }
}

void AFortPlayerController::ServerEditBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, int32 RotationIterations, bool bMirrored)
{
    if (!BuildingActorToEdit || !BuildingActorToEdit->GetWorld() || (APlayerState*)BuildingActorToEdit->GetEditingPlayer() != GetPlayerState() || BuildingActorToEdit->GetDestroyed())
        return;

    BuildingActorToEdit->SetEditingPlayer(nullptr);
    BuildingActorToEdit->ReplaceBuildingActor(EBuildingReplacementType::BRT_Edited, NewBuildingClass, BuildingActorToEdit->GetCurrentBuildingLevel(), RotationIterations, bMirrored, this);

    UFortAnalytics::FireEvent_BuildingAction(this, TEXT("Edit"), BuildingActorToEdit, 0);

    if (auto GameMode = Cast<AFortGameMode>(GetWorld()->GetAuthGameMode()))
        GameMode->OnPlayerEditedBuilding(this, BuildingActorToEdit);
}

void AFortPlayerController::ServerEndEditingBuildingActor_Implementation(ABuildingSMActor* BuildingActorToStopEditing)
{
    if (!BuildingActorToStopEditing || !GetMyFortPawn() || (APlayerState*)BuildingActorToStopEditing->GetEditingPlayer() != GetPlayerState() || BuildingActorToStopEditing->GetDestroyed())
        return;

    BuildingActorToStopEditing->SetEditingPlayer(nullptr);

    auto EditToolItemDefinition = UFortGameData::Get()->GetEditToolItem();
    auto EditToolItem = K2_FindExistingItemForDefinition(EditToolItemDefinition, false);

    if (EditToolItem && EditToolItemDefinition && EditToolItemDefinition->ServerExecute((UFortWorldItem*)EditToolItem, this))
    {
        if (auto EditTool = Cast<AFortWeap_EditingTool>(GetMyFortPawn()->GetCurrentWeapon()))
            EditTool->SetEditActor(nullptr);
    }
}

void AFortPlayerController::ServerRepairBuildingActor_Implementation(ABuildingSMActor* BuildingActorToRepair)
{
    if (!BuildingActorToRepair || !BuildingActorToRepair->GetWorld())
        return;

    static auto NoEditPermissionText = (const FText*)ClientRepair().Find("lea rdx, [0x*]").Target(TEXT("NoEditPermissionText"));
    static auto BuildingLocked = (const FText*)ClientRepair().Find("lea rdx, [0x*]", 2).Target(TEXT("BuildingLocked"));
    static auto UnableToAffordRepair = (const FText*)ClientRepair().Find("lea rdx, [0x*]", 3).Target(TEXT("UnableToAffordRepair"));

    const bool bNeedsRepair = BuildingActorToRepair->NeedsRepair();
    const FRotator Rotation = BuildingActorToRepair->GetActorRotation();
    const FVector Location = BuildingActorToRepair->GetActorLocation();
    const bool bHasPermissionToEditWorld = HasPermissionToEditWorld(BuildingActorToRepair, Location, Rotation);

    if (!bNeedsRepair && !bHasPermissionToEditWorld)
    {
        ClientSendMessage(*NoEditPermissionText);
        TriggerUIFeedbackEvent(FName(TEXT("BuildPreviewUnableToPlace")));
        return;
    }

    if ((!bNeedsRepair || bHasPermissionToEditWorld) && BuildingActorToRepair->IsBeingEdited())
    {
        ClientSendMessage(*BuildingLocked);
        TriggerUIFeedbackEvent(FName(TEXT("BuildPreviewUnableToPlace")));
        return;
    }

    if (!bNeedsRepair)
        return;

    if (!CanAffordToRepair(BuildingActorToRepair))
    {
        ClientSendMessage(*UnableToAffordRepair);
        TriggerUIFeedbackEvent(FName(TEXT("BuildPreviewUnableToAfford")));
        return;
    }

    BuildingActorToRepair->RepairBuilding(this, PayBuildingRepairCost(BuildingActorToRepair));

    if (auto GameMode = Cast<AFortGameMode>(GetWorld()->GetAuthGameMode()))
        GameMode->OnPlayerRepairedBuilding(this, BuildingActorToRepair);

    if (auto MyFortPawn = GetMyFortPawn())
        UFortAIFunctionLibrary::MakeNoiseEventAtLocation(MyFortPawn, 1000.0f, BuildingActorToRepair->GetActorLocation());

    if (auto StartRepairSound = GetStartRepairSound().LoadSynchronous())
        UFortKismetLibrary::BroadcastSoundAtLocation(this, (USoundBase*)StartRepairSound, BuildingActorToRepair->GetActorLocation(), 1.0f, 1.0f);
}

bool AFortPlayerController::ValidateBuildingClassData(FBuildingClassData& BuildingClassData) const
{
    if (!BuildingClassData.BuildingClass || !BuildingClassData.BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return false;

    auto BuildingSMActor = (ABuildingSMActor*)BuildingClassData.BuildingClass->GetDefaultObject();

    if (!BuildingSMActor)
        return false;

    if (BuildingSMActor->GetCurrentBuildingLevel() <= 0)
        return true;

    auto GameState = Cast<AFortGameStateZone>(GetWorld()->GetGameState());

    if (!GameState)
        return false;

    FPlayerBuildableClassFilter Filter{ BuildingSMActor->GetResourceType(), BuildingSMActor->GetBuildingType(), 0, BuildingSMActor->GetEditModePatternData() };
    TArray<UClass*> BuildingClasses;

    GameState->GetPlayerBuildableClasses(BuildingClasses, Filter);

    if (BuildingClasses.Num() == 0)
    {
        Filter.EditModePatternData = nullptr;
        GameState->GetPlayerBuildableClasses(BuildingClasses, Filter);
    }

    auto BuildingClass = BuildingClasses.Num() > 0 && BuildingClasses[0] && BuildingClasses[0]->IsChildOf(ABuildingActor::StaticClass()) ? BuildingClasses[0] : nullptr;

    if (!BuildingClass || !BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return false;

    BuildingClassData.PreviousBuildingLevel = -1;
    BuildingClassData.UpgradeLevel = 0;
    BuildingClassData.BuildingClass = BuildingClass;

    return true;
}

bool AFortPlayerController::CanPerformNativeAction(const FGameplayTag& ActionTag) const
{
    static auto CanPerformNativeAction = []
    {
        auto Call = ClientBuild().Find("call 0x*; mov rcx, *; lea rdx, [rax+0x*]; call [*]").Find("call [*]");

        return Scanner::Slot(TEXT("AFortPlayerController::CanPerformNativeAction"), Call.Disp() ? Call.Disp() : (int32)Call.Last("add *, 0x*").Imm());
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AFortPlayerController*, const FGameplayTag&))(VTable + CanPerformNativeAction))(this, ActionTag);
}

bool AFortPlayerController::IsBuildingRestricted(const FBuildingClassData& BuildingClassData, const FVector& BuildLoc, const FRotator& BuildRot) const
{
    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AFortPlayerController*, const FBuildingClassData&, const FVector&, const FRotator&))(VTable + IsBuildingRestrictedSlot()))(this, BuildingClassData, BuildLoc, BuildRot);
}

bool AFortPlayerController::HasRequiredBuildingLevel(const FBuildingClassData& BuildingClassData) const
{
    if (!BuildingClassData.BuildingClass || !BuildingClassData.BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return false;

    auto BuildingSMActor = (ABuildingSMActor*)BuildingClassData.BuildingClass->GetDefaultObject();

    if (!BuildingSMActor)
        return false;

    return HasRequiredBuildingLevel(BuildingClassData.UpgradeLevel, BuildingSMActor->GetResourceType());
}

bool AFortPlayerController::HasRequiredBuildingLevel(int32 BuildingLevel, EFortResourceType ResourceType) const
{
    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AFortPlayerController*, int32, EFortResourceType))(VTable + IsBuildingRestrictedSlot() + sizeof(void*)))(this, BuildingLevel, ResourceType);
}

bool AFortPlayerController::CanAffordToPlaceBuildableClass(FBuildingClassData BuildingClassData) const
{
    static auto CanAffordToPlaceBuildableClass = ClientBuild().Find("lea rdx, [0x*]; lea rcx, *; call 0x*").Find("call [*+0x*]").Slot(TEXT("AFortPlayerController::CanAffordToPlaceBuildableClass"));

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AFortPlayerController*, FBuildingClassData))(VTable + CanAffordToPlaceBuildableClass))(this, BuildingClassData);
}

EFortStructuralGridQueryResults AFortPlayerController::CanPlaceBuildableClassInStructuralGrid(UClass* BuildingClass, FVector& BuildLoc, FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings)
{
    static auto CanPlaceBuildableClassInStructuralGrid = []
    {
        auto Call = Scanner::String(TEXT("Failed to find intersection point!")).Start().Find("call [r10+0x*]");

        return Scanner::Slot(TEXT("AFortPlayerController::CanPlaceBuildableClassInStructuralGrid"), Call.Get() ? Call.Disp() : IsBuildingRestrictedSlot() + 10 * (int32_t)sizeof(void*));
    }();

    auto VTable = *(uint8_t**)this;

    return (*(EFortStructuralGridQueryResults (**)(AFortPlayerController*, UClass*, FVector&, FRotator&, bool, TArray<ABuildingActor*>*))(VTable + CanPlaceBuildableClassInStructuralGrid))(this, BuildingClass, BuildLoc, BuildRot, bMirrored, ExistingBuildings);
}

int32 AFortPlayerController::PayBuildableClassPlacementCost(FBuildingClassData BuildingClassData)
{
    static auto PayBuildableClassPlacementCost = PlayerControllerSlot(Scanner::String(TEXT("Failed to remove item %s during pay building costs, item duplicated!")).Start().Get(TEXT("AFortPlayerController::PayBuildableClassPlacementCost")));

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(AFortPlayerController*, FBuildingClassData))(VTable + PayBuildableClassPlacementCost))(this, BuildingClassData);
}

FText AFortPlayerController::GetErrorMessageFromStructuralQuery(EFortStructuralGridQueryResults QueryResults) const
{
    static auto GetErrorMessageFromStructuralQuery = ClientBuild().Find("mov r8d, *; *; call [*+0x*]; mov rdx, rax").Find("call [*+0x*]").Slot(TEXT("AFortPlayerController::GetErrorMessageFromStructuralQuery"));

    auto VTable = *(uint8_t**)this;
    FText Result(NoInit);

    (*(FText* (**)(const AFortPlayerController*, FText*, EFortStructuralGridQueryResults))(VTable + GetErrorMessageFromStructuralQuery))(this, &Result, QueryResults);

    return Result;
}

bool AFortPlayerController::HasPermissionToEditWorld(ABuildingActor* BuildingActor, const FVector& Location, const FRotator& Rotation) const
{
    static auto HasPermissionToEditWorld = ClientRepair().Find("call [*+0x*]", 2).Slot(TEXT("AFortPlayerController::HasPermissionToEditWorld"));

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AFortPlayerController*, ABuildingActor*, const FVector&, const FRotator&))(VTable + HasPermissionToEditWorld))(this, BuildingActor, Location, Rotation);
}

bool AFortPlayerController::CanAffordToRepair(ABuildingSMActor* BuildingActorToRepair) const
{
    static auto CanAffordToRepair = ClientRepair().Find("call [*+0x*]", 5).Slot(TEXT("AFortPlayerController::CanAffordToRepair"));

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AFortPlayerController*, ABuildingSMActor*))(VTable + CanAffordToRepair))(this, BuildingActorToRepair);
}

int32 AFortPlayerController::PayBuildingRepairCost(ABuildingSMActor* BuildingActorToRepair)
{
    static auto PayBuildingRepairCost = PlayerControllerSlot(Scanner::String(TEXT("Failed to remove item %s during repair, item duplicated!")).Start().Get(TEXT("AFortPlayerController::PayBuildingRepairCost")));

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(AFortPlayerController*, ABuildingSMActor*))(VTable + PayBuildingRepairCost))(this, BuildingActorToRepair);
}

int32 AFortPlayerController::GetWorldPlayerId() const
{
    auto PlayerState = Cast<AFortPlayerState>(GetPlayerState());

    return PlayerState ? PlayerState->GetWorldPlayerId() : -1;
}

void AFortPlayerController::ClientSendMessage(const FText& MESSAGE)
{
    static auto ClientSendMessage = Scanner::Function<void (*)(AFortPlayerController*, const FText&)>(Scanner::Event(TEXT("ClientSendMessage")).Get(TEXT("AFortPlayerController::ClientSendMessage")));

    ClientSendMessage(this, MESSAGE);
}

void AFortPlayerController::ClientExecuteInventoryItem(const FGuid& ItemGuid, float Delay, bool bForceExecute, bool bActivateSlotAfterSettingFocused)
{
    static auto ClientExecuteInventoryItem = Scanner::Function<void (*)(AFortPlayerController*, const FGuid&, float, bool, bool)>(Scanner::Event(TEXT("ClientExecuteInventoryItem")).Get(TEXT("AFortPlayerController::ClientExecuteInventoryItem")));

    ClientExecuteInventoryItem(this, ItemGuid, Delay, bForceExecute, bActivateSlotAfterSettingFocused);
}

void AFortPlayerController::TriggerUIFeedbackEvent(FName EventName)
{
    static auto TriggerUIFeedbackEvent = Scanner::Function<void (*)(AFortPlayerController*, FName)>(Scanner::CallTo(Scanner::Event(TEXT("ClientTriggerUIFeedbackEvent")).Get()).Start().Get(TEXT("AFortPlayerController::TriggerUIFeedbackEvent")));

    TriggerUIFeedbackEvent(this, EventName);
}

AFortPlayerPawn* AFortPlayerController::GetMyFortPawn() const
{
    static auto MyFortPawn = GetClass()->FindPropertyByName(FName(TEXT("MyFortPawn")));

    return *MyFortPawn->ContainerPtrToValuePtr<AFortPlayerPawn*>(this);
}

TAssetPtr<UObject>& AFortPlayerController::GetStartRepairSound()
{
    static auto StartRepairSound = GetClass()->FindPropertyByName(FName(TEXT("StartRepairSound")));

    return *StartRepairSound->ContainerPtrToValuePtr<TAssetPtr<UObject>>(this);
}

void AFortPlayerController::ResetQuickBarToPrimary()
{
    static auto ResetQuickBarToPrimary = []
    {
        auto ClientOnPawnDied = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerControllerZone:ClientOnPawnDied")).Find("call [*+0x*]").Slot(TEXT("AFortPlayerControllerZone::ClientOnPawnDied_Implementation"));
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"));

        return Scanner::Function<void (*)(AFortPlayerController*)>(Scanner::At(VTable[ClientOnPawnDied / sizeof(void*)]).Call(2).Get(TEXT("AFortPlayerController::ResetQuickBarToPrimary")));
    }();

    ResetQuickBarToPrimary(this);
}

UStatManager* AFortPlayerController::GetStatManager() const
{
    static auto StatManager = GetClass()->FindPropertyByName(FName(TEXT("StatManager")));

    return *StatManager->ContainerPtrToValuePtr<UStatManager*>(this);
}
