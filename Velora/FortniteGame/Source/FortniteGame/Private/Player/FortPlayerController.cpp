#include "pch.h"
#include "Player/FortPlayerController.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/Cosmetics/AthenaDanceItemDefinition.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Athena/Cosmetics/AthenaSprayItemDefinition.h"
#include "Athena/Cosmetics/AthenaToyItemDefinition.h"
#include "Player/FortPlayerState.h"
#include "Player/FortPlayerStateZone.h"
#include "Building/BuildingSMActor.h"
#include "Items/FortInventory.h"
#include "Items/FortMontageItemDefinitionBase.h"
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
#include "Abilities/FortGameplayAbility.h"
#include "AI/FortAIFunctionLibrary.h"
#include "Analytics/FortAnalytics.h"
#include "FortGameData.h"
#include "FortGameMode.h"
#include "FortGameStateZone.h"
#include "FortKismetLibrary.h"
#include "FortniteGame.h"
#include "AbilitySystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "Math/TransformVectorized.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

bool AFortPlayerController::InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized, bool bForceSync)
{
    DETOUR(AFortPlayerController, InitializeMcpProfile, Anchor::Hook()
        .String(TEXT("Initialized MCP profile for %s*"))
        .Start()
        .Inline());

    static auto ServerCreateBuildingActor = THook(TEXT("AFortPlayerController::ServerCreateBuildingActor_Implementation"), (void (AFortPlayerController::*)(const FCreateBuildingActorData&))&AFortPlayerController::ServerCreateBuildingActor_Implementation)
        .If(((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ServerCreateBuildingActor")))->FindPropertyByName(FName(TEXT("CreateBuildingData"))) != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerCreateBuildingActor"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerCreateBuildingActor_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerCreateBuildingActorClassData = THook(TEXT("AFortPlayerController::ServerCreateBuildingActor_Implementation"), (void (AFortPlayerController::*)(FBuildingClassData, const TGameValue<FVector>&, const TGameValue<FRotator>&, bool))&AFortPlayerController::ServerCreateBuildingActor_Implementation)
        .If(!((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ServerCreateBuildingActor")))->FindPropertyByName(FName(TEXT("CreateBuildingData"))))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerCreateBuildingActor"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerCreateBuildingActor_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerBeginEditingBuildingActor = THook(TEXT("AFortPlayerController::ServerBeginEditingBuildingActor_Implementation"), &AFortPlayerController::ServerBeginEditingBuildingActor_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerBeginEditingBuildingActor"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerBeginEditingBuildingActor_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerEditBuildingActor = THook(TEXT("AFortPlayerController::ServerEditBuildingActor_Implementation"), &AFortPlayerController::ServerEditBuildingActor_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerEditBuildingActor"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerEditBuildingActor_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerEndEditingBuildingActor = THook(TEXT("AFortPlayerController::ServerEndEditingBuildingActor_Implementation"), &AFortPlayerController::ServerEndEditingBuildingActor_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerEndEditingBuildingActor"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerEndEditingBuildingActor_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerRepairBuildingActor = THook(TEXT("AFortPlayerController::ServerRepairBuildingActor_Implementation"), &AFortPlayerController::ServerRepairBuildingActor_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRepairBuildingActor"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerRepairBuildingActor_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerAcknowledgePossession = THook(TEXT("AFortPlayerControllerZone::ServerAcknowledgePossession_Implementation"), &AFortPlayerControllerZone::ServerAcknowledgePossession_Implementation)
        .Exec(TEXT("/Script/Engine.PlayerController:ServerAcknowledgePossession"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("APlayerController::ServerAcknowledgePossession_Implementation"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerExecuteInventoryItem = THook(TEXT("AFortPlayerController::ServerExecuteInventoryItem_Implementation"), &AFortPlayerController::ServerExecuteInventoryItem_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerExecuteInventoryItem"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerExecuteInventoryItem_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerSpawnInventoryDrop = THook(TEXT("AFortPlayerController::ServerSpawnInventoryDrop_Implementation"), &AFortPlayerController::ServerSpawnInventoryDrop_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerSpawnInventoryDrop"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerSpawnInventoryDrop_Implementation"))
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerAttemptInventoryDrop"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerSpawnInventoryDrop_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerRemoveInventoryStateValue = THook(TEXT("AFortPlayerController::ServerRemoveInventoryStateValue_Implementation"), &AFortPlayerController::ServerRemoveInventoryStateValue_Implementation)
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryStateValue"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerRemoveInventoryStateValue_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto bGameServerPlayEmoteItem = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerPlayEmoteItem"))
        .Find("call [rax+*]", 2)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"), "test rdx, rdx; jz *")
        .HasVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"), "mov rdx, rbx; mov rcx, rdi; call 0x*")
        .Get();

    static auto ServerPlayEmoteItem = THook(TEXT("AFortPlayerController::ServerPlayEmoteItem_Implementation"), &AFortPlayerController::ServerPlayEmoteItem_Implementation)
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ServerPlayEmoteItem")))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerPlayEmoteItem"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerPlayEmoteItem_Implementation"))
        .If(!bGameServerPlayEmoteItem)
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    static auto ServerRemoveItemInternal = THook(TEXT("AFortQuickBars::ServerRemoveItemInternal_Implementation"), &AFortQuickBars::ServerRemoveItemInternal_Implementation)
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuickBars:ServerRemoveItemInternal")))
        .Exec(TEXT("/Script/FortniteGame.FortQuickBars:ServerRemoveItemInternal"))
        .Find("call [*+0x*]", 2)
        .LogSlot(TEXT("AFortQuickBars::ServerRemoveItemInternal_Implementation"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortQuickBars"));

    static auto InventoryOwnerOffset = Anchor::Offset(TEXT("AFortPlayerController::IFortInventoryOwnerInterface"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerController::ServerRemoveInventoryItem_Implementation"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
        .Find("call [*+0x*]")
        .Last("add rcx, 0x*")
        .Imm();

    static auto RemoveInventoryItem = THook(TEXT("AFortPlayerController::RemoveInventoryItem"), (void*)+[](uint8_t* InventoryOwnerInterface, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars)
        {
            return ((AFortPlayerController*)(InventoryOwnerInterface - InventoryOwnerOffset))->RemoveInventoryItem(ItemGuid, Count, bForceRemoveFromQuickBars);
        })
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem"))
        .Find("call [rax+*]", 2)
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("IFortInventoryOwnerInterface::RemoveInventoryItem"))
        .InVTable(*(void***)((uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena")) + InventoryOwnerOffset));

    static auto SetLoadedAmmo = THook(TEXT("UFortWorldItem::SetLoadedAmmo"), &UFortWorldItem::SetLoadedAmmo)
        .VirtualStartHas(*(const void* const* const*)((uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena")) + InventoryOwnerOffset), "jmp [r8+*]")
        .Start()
        .Find("jmp [r8+*]")
        .LogSlot(TEXT("UFortWorldItem::SetLoadedAmmo"))
        .Or()
        .VirtualStartHas(*(const void* const* const*)((uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena")) + InventoryOwnerOffset), "call [rax+0x18]; test rax, rax; jz *; mov r8, [rax]; mov edx, *; mov rcx, rax; call [r8+0x*]")
        .Start()
        .Find("call [r8+*]")
        .LogSlot(TEXT("UFortWorldItem::SetLoadedAmmo"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortWorldItem"));

    auto bInitialized = Original(OnMcpProfilesInitialized, bForceSync);

    static auto bServerQuickBars = GetClass()->FindPropertyByName(FName(TEXT("QuickBars"))) != nullptr;

    if (HasAuthority() && bServerQuickBars && !GetQuickBars())
    {
        FActorSpawnParameters SpawnInfo;
        SpawnInfo.Owner = this;

        SetQuickBars((AFortQuickBars*)GetWorld()->SpawnActor(AFortQuickBars::StaticClass(), nullptr, SpawnInfo));
    }

    return bInitialized;
}

void AFortPlayerController::TryAddToQuickBar(UFortItem* Item)
{
    static auto TryAddToQuickBar = Anchor::VirtualFunction<void (*)(AFortPlayerController*, UFortItem*)>(TEXT("AFortPlayerController::TryAddToQuickBar"))
        .String(TEXT("Failed TryAddToQuickBar due to invalid ItemDefinition for Item '%s'"))
        .Start()
        .Log(TEXT("AFortPlayerController::TryAddToQuickBar"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
        .Optional()
        .Get();

    if (TryAddToQuickBar)
        return TryAddToQuickBar(this, Item);

    static auto NativeTryAddToQuickBar = Anchor::Function<void (*)(AFortPlayerController*, UFortItem*)>(TEXT("AFortPlayerController::TryAddToQuickBar"))
        .String(TEXT("Failed TryAddToQuickBar due to invalid ItemDefinition for Item '%s'"))
        .Start()
        .Get();

    NativeTryAddToQuickBar(this, Item);
}

bool AFortPlayerController::CanServerTryAddToQuickBar() const
{
    static auto bAuthorityCheck = Anchor::Exists()
        .String(TEXT("Failed TryAddToQuickBar due to invalid ItemDefinition for Item '%s'"))
        .Start()
        .Has("cmp byte ptr [rcx+0x*], 0x3; mov rdi, rdx")
        .Get();

    static auto bResultCheck = Anchor::Exists()
        .String(TEXT("Failed TryAddToQuickBar due to invalid ItemDefinition for Item '%s'"))
        .Start()
        .Has("call 0x*; cmp eax, 0x1; jz 0x*")
        .Or()
        .String(TEXT("Failed TryAddToQuickBar due to invalid ItemDefinition for Item '%s'"))
        .Start()
        .Has("mov eax, *; cmp eax, 0x1; jz 0x*")
        .Or()
        .String(TEXT("Failed TryAddToQuickBar due to invalid ItemDefinition for Item '%s'"))
        .Start()
        .Has("call 0x*; cmp eax, 0x1; jmp 0x*")
        .Get();

    return !bAuthorityCheck && !bResultCheck;
}

void AFortPlayerController::RemoveAllInventoryItems()
{
    DETOUR(AFortPlayerController, RemoveAllInventoryItems, Anchor::Hook()
        .String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase"))
        .Find("mov *[rsp+0x20], *; xor r8d, r8d; xor edx, edx; mov rcx, *; call 0x*")
        .Find("call 0x*", 3)
        .IsCallTo(Anchor::Address()
            .String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase"))
            .Find("mov *[rsp+0x20], *; xor r8d, r8d; xor edx, edx; mov rcx, *; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get())
        .Optional()
        .AtCall());

    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode());
    auto PlayerController = Cast<AFortPlayerControllerAthena>(this);
    auto CustomizationLoadout = PlayerController ? PlayerController->GetCustomizationLoadout() : nullptr;
    auto Pickaxe = CustomizationLoadout ? CustomizationLoadout->GetPickaxe() : nullptr;
    auto& ReplicatedEntries = WorldInventory->GetInventory().GetReplicatedEntries();

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

void AFortPlayerController::ClientActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot, bool bForceExecution)
{
    DETOUR(AFortPlayerController, ClientActivateSlot, Anchor::Hook()
        .String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase"))
        .Find("mov byte ptr [rsp+0x20], 0x1; xor edx, edx; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Or()
        .String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase"))
        .Find("xorps xmm3, xmm3; xor r8d, r8d; mov [rsp+0x20], *; xor edx, edx; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Optional()
        .AtCall());

    static auto ClientActivateSlot = Anchor::Function<void (*)(AFortPlayerController*, EFortQuickBars, int32, float, bool, bool)>(TEXT("AFortPlayerController::ClientActivateSlot"))
        .Event(TEXT("ClientActivateSlot"))
        .Get();

    ClientActivateSlot(this, InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot, bForceExecution);

    if (!DETOUR_ORIGINAL(AFortPlayerController, RemoveAllInventoryItems))
        RemoveAllInventoryItems();
}

void AFortPlayerController::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    DETOUR(AFortPlayerController, ActivateSlot, Anchor::Hook()
        .After(DETOUR_ORIGINAL(AFortPlayerController, RemoveAllInventoryItems))
        .If(!DETOUR_ORIGINAL(AFortPlayerController, ClientActivateSlot))
        .String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase"))
        .Find("mov *[rsp+0x20], *; xor r8d, r8d; xor edx, edx; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .AtCall());

    if (GetQuickBars())
        GetQuickBars()->ActivateSlot(InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot);

    if (!DETOUR_ORIGINAL(AFortPlayerController, RemoveAllInventoryItems))
        RemoveAllInventoryItems();
}

UFortItem* AFortPlayerController::K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:BP_FindExistingItemForDefinition"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto InventoryOwner = Anchor::Offset(TEXT("AFortPlayerController::BP_FindExistingItemForDefinition inventory owner"))
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:BP_FindExistingItemForDefinition"))
            .Find("lea rcx, [*+0x*]; mov rdx, *; setnz r8b")
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:BP_FindExistingItemForDefinition"))
            .Find("lea rcx, [*+0x*]; movups xmm0, [*]; setnz r8b")
            .Disp();

        static auto FindExistingItemForDefinition = Anchor::VirtualFunction<UFortItem* (*)(const void*, const UFortItemDefinition*, bool)>(TEXT("IFortInventoryOwnerInterface::FindExistingItemForDefinition"))
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:BP_FindExistingItemForDefinition"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [rax+0x*]")
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:BP_FindExistingItemForDefinition"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("mov r9, [rax+0x*]")
            .Get();

        static auto bItemVariantGuid = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:BP_FindExistingItemForDefinition")))->FindPropertyByName(FName(TEXT("ItemVariantGuid"))) != nullptr;

        if (!InventoryOwner)
            return nullptr;

        if (bItemVariantGuid)
        {
            struct
            {
                const UFortItemDefinition* ItemDefinition;
                FGuid ItemVariantGuid;
            } const ItemAndVariant{ ItemDefinition, FGuid() };

            return FindExistingItemForDefinition.As<UFortItem* (*)(const void*, const void*, bool)>()((const uint8*)this + InventoryOwner, &ItemAndVariant, bInStorageVault);
        }

        return FindExistingItemForDefinition((const uint8*)this + InventoryOwner, ItemDefinition, bInStorageVault);
    }

    static auto K2_FindExistingItemForDefinition = Anchor::Function<UFortItem* (*)(const AFortPlayerController*, const UFortItemDefinition*, bool)>(TEXT("AFortPlayerController::K2_FindExistingItemForDefinition"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:K2_FindExistingItemForDefinition"))
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:BP_FindExistingItemForDefinition"))
        .Get();

    return K2_FindExistingItemForDefinition(this, ItemDefinition, bInStorageVault);
}

void AFortPlayerController::ServerExecuteInventoryItem_Implementation(FGuid ItemGuid)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    for (auto Item : WorldInventory->GetInventory().GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
        {
            if (auto MyFortPawn = GetMyFortPawn())
                MyFortPawn->SetLastEquippedWeaponOrGadget(Item);

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

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
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
        {
            Pickup->SetPawnWhoDroppedPickup(Pawn);
            Pickup->TossPickup(Location, Pawn, 0, true);
        }

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

    for (auto& ItemEntry : Inventory.GetReplicatedEntries())
    {
        if (ItemEntry.GetItemGuid() != ItemGuid)
            continue;

        ItemEntry.RemoveStateValue(StateValueType);
        Inventory.MarkItemDirty(ItemEntry);
        WorldInventory->HandleInventoryLocalUpdate();

        return;
    }
}

void AFortPlayerController::ServerPlayEmoteItem_Implementation(UFortMontageItemDefinitionBase* EmoteAsset)
{
    if (!EmoteAsset)
        return;

    EmoteAsset->GetAnimation().LoadSynchronous();

    auto MyFortPawn = GetMyFortPawn();
    auto AbilitySystemComponent = MyFortPawn ? MyFortPawn->GetAbilitySystemComponent() : nullptr;

    if (!AbilitySystemComponent || !CanPerformNativeAction(FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.InteractUse")))))
        return;

    const TAssetPtr<UClass>* AbilityToGrant = nullptr;

    if (auto ToyAsset = Cast<UAthenaToyItemDefinition>(EmoteAsset))
        AbilityToGrant = ToyAsset->GetToySpawnAbility();
    else if (Cast<UAthenaSprayItemDefinition>(EmoteAsset))
        AbilityToGrant = UFortGameData::Get()->GetSprayGameplayAbility();
    else if (auto DanceAsset = Cast<UAthenaDanceItemDefinition>(EmoteAsset))
        AbilityToGrant = DanceAsset->GetCustomDanceAbility();

    if (!AbilityToGrant || AbilityToGrant->IsNull())
        AbilityToGrant = &UFortGameData::Get()->GetEmoteGameplayAbility();

    auto EmoteGameplayAbility = AbilityToGrant->LoadSynchronous();

    if (!EmoteGameplayAbility || !EmoteGameplayAbility->IsChildOf(UFortGameplayAbility::StaticClass()))
        return;

    AbilitySystemComponent->GiveAbilityAndActivateOnce(FGameplayAbilitySpec(EmoteGameplayAbility->GetDefaultObject<UFortGameplayAbility>(), 1, INDEX_NONE, EmoteAsset));

    auto PreviousEmote = MyFortPawn->GetLastReplicatedEmoteExecuted();

    MyFortPawn->SetLastReplicatedEmoteExecuted(EmoteAsset);
    MyFortPawn->OnRep_LastReplicatedEmoteExecuted(PreviousEmote);

    static auto bHasMovingEmote = AFortPawn::StaticClass()->FindPropertyByName(FName(TEXT("bMovingEmote"))) != nullptr;

    if (auto DanceAsset = bHasMovingEmote ? Cast<UAthenaDanceItemDefinition>(EmoteAsset) : nullptr)
    {
        MyFortPawn->SetMovingEmote(DanceAsset->GetMovingEmote());
        MyFortPawn->SetMovingEmoteForwardOnly(DanceAsset->GetMoveForwardOnly());
        MyFortPawn->SetMovingEmoteFollowingOnly(DanceAsset->GetMoveFollowingOnly());
        MyFortPawn->SetMovingEmoteSkipLandingFX(DanceAsset->GetMovingEmoteSkipLandingFX());
        MyFortPawn->SetEmoteWalkSpeed(DanceAsset->GetWalkForwardSpeed());
    }
}

AActor* AFortPlayerController::SpawnToyInstance(UClass* ToyClass, const FTransform& SpawnPosition)
{
    if (!ToyClass || !HasAuthority())
        return nullptr;

    auto ToyInstance = GetWorld()->SpawnActor(ToyClass, &SpawnPosition);

    if (!ToyInstance)
        return nullptr;

    auto& NumTimesSummoned = GetToySummonCounts().FindOrAdd(ToyClass);
    NumTimesSummoned++;

    GetActiveToyInstances().Add(ToyInstance);

    auto PlayerState = Cast<AFortPlayerStateZone>(GetPlayerState());

    if (auto OwningPawn = ToyClass->FindPropertyByName(FName(TEXT("OwningPawn"))))
        *OwningPawn->ContainerPtrToValuePtr<APawn*>(ToyInstance) = PlayerState ? PlayerState->GetCurrentPawn() : nullptr;

    if (auto InitialSpawnPosition = ToyClass->FindPropertyByName(FName(TEXT("InitialSpawnPosition"))))
        InitialSpawnPosition->SetPropertyValue_InContainer<FVector>(ToyInstance, ToyInstance->GetActorLocation());

    if (auto ProjectileMovement = ToyClass->FindPropertyByName(FName(TEXT("ProjectileMovement"))))
    {
        if (auto ProjectileMovementComponent = *ProjectileMovement->ContainerPtrToValuePtr<UObject*>(ToyInstance))
        {
            if (auto Velocity = ProjectileMovementComponent->GetClass()->FindPropertyByName(FName(TEXT("Velocity"))))
                Velocity->SetPropertyValue_InContainer<FVector>(ProjectileMovementComponent, FVector(0.f, 0.f, 0.f));
        }
    }

    if (auto HasFinishedSpawning = (UBoolProperty*)ToyClass->FindPropertyByName(FName(TEXT("HasFinishedSpawning"))))
        HasFinishedSpawning->SetPropertyValue_InContainer(ToyInstance, true);

    if (auto BallSerialNumber = ToyClass->FindPropertyByName(FName(TEXT("BallSerialNumber"))))
        *BallSerialNumber->ContainerPtrToValuePtr<int32>(ToyInstance) = NumTimesSummoned;

    if (auto StrokeCount = ToyClass->FindPropertyByName(FName(TEXT("StrokeCount"))))
        *StrokeCount->ContainerPtrToValuePtr<int32>(ToyInstance) = 1;

    if (auto StartedAtTeeBoxNumber = ToyClass->FindPropertyByName(FName(TEXT("StartedAtTeeBoxNumber"))))
    {
        static auto B_GolfMinigame_TeeBox_C = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Abilities/Toys/Golf/Minigame/B_GolfMinigame_TeeBox.B_GolfMinigame_TeeBox_C"));
        static auto HoleNumber = B_GolfMinigame_TeeBox_C ? B_GolfMinigame_TeeBox_C->FindPropertyByName(FName(TEXT("HoleNumber"))) : nullptr;
        auto SphereCollision = ToyClass->FindPropertyByName(FName(TEXT("SphereCollision")));
        auto SphereCollisionComponent = SphereCollision ? *SphereCollision->ContainerPtrToValuePtr<UPrimitiveComponent*>(ToyInstance) : nullptr;

        if (HoleNumber && SphereCollisionComponent)
        {
            TArray<AActor*> OverlappingActors;
            SphereCollisionComponent->GetOverlappingActors(OverlappingActors, B_GolfMinigame_TeeBox_C);

            for (auto TeeBox : OverlappingActors)
                *StartedAtTeeBoxNumber->ContainerPtrToValuePtr<int32>(ToyInstance) = *HoleNumber->ContainerPtrToValuePtr<int32>(TeeBox);
        }
    }

    if (auto OwningPC = ToyClass->FindPropertyByName(FName(TEXT("OwningPC"))))
        *OwningPC->ContainerPtrToValuePtr<AFortPlayerController*>(ToyInstance) = this;

    return ToyInstance;
}

void AFortPlayerController::execSpawnToyInstance(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerController, execSpawnToyInstance, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:SpawnToyInstance"))
        .Is("xor eax, eax; ret")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerController:SpawnToyInstance"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerController:SpawnToyInstance")));

    UClass* ToyClass = nullptr;
    TGameValue<FTransform> SpawnPosition;

    Stack.StepCompiledIn(&ToyClass);
    Stack.StepCompiledIn(&SpawnPosition);
    Stack.Code += !!Stack.Code;

    *(AActor**)Z_Param__Result = SpawnToyInstance(ToyClass, SpawnPosition.Get());
}

TArray<AActor*>& AFortPlayerController::GetActiveToyInstances()
{
    static auto ActiveToyInstances = GetClass()->FindPropertyByName(FName(TEXT("ActiveToyInstances")));

    return *ActiveToyInstances->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

TMap<UClass*, int32>& AFortPlayerController::GetToySummonCounts()
{
    static auto ToySummonCounts = GetClass()->FindPropertyByName(FName(TEXT("ToySummonCounts")));

    return *ToySummonCounts->ContainerPtrToValuePtr<TMap<UClass*, int32>>(this);
}

void AFortPlayerController::DropItemsOnPawnDestruction(AActor* DamageCauser, const FGameplayTagContainer& DeathTags, AFortPlayerPawn* DestroyedPawn, bool& bDroppedItems)
{
    DETOUR(AFortPlayerController, DropItemsOnPawnDestruction, Anchor::Hook()
        .String(TEXT("Deaths"))
        .Start()
        .Find("mov byte ptr [rsp+0x*], 0x0; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Or()
        .String(TEXT("Deaths"))
        .Start()
        .Find("mov *[r?p+0x*], *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("AFortPlayerController::DropItemsOnPawnDestruction"))
        .AtCall(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena")));

    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory || !DestroyedPawn)
        return;

    auto& ReplicatedEntries = WorldInventory->GetInventory().GetReplicatedEntries();
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

void AFortPlayerController::DropItemsAsPickups(const TArray<UFortWorldItem*>& Items, AFortPawn* Pawn, AFortInventory* Inventory, AFortPlayerController* PlayerController)
{
    DETOUR(AFortPlayerController, DropItemsAsPickups, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortAthenaAIBotController, DropInventoryItems))
        .FieldPattern("80 ? %o 00", AFortPlayerPawnAthena::StaticClass()->FindPropertyByName(FName(TEXT("bShouldDropItemsOnDeath"))))
        .Optional()
        .Call()
        .Find("xor r9d, r9d; call 0x*")
        .Find("call 0x*")
        .Or()
        .FieldPattern("80 ? %o 00", AFortPlayerPawnAthena::StaticClass()->FindPropertyByName(FName(TEXT("bShouldDropItemsOnDeath"))))
        .Call()
        .Find("xor r9d, r9d; mov dword ptr [rsp+0x20], *; call 0x*")
        .Find("call 0x*")
        .AtCall());

    if (!Pawn)
        return;

    const FVector Location = Pawn->GetActorLocation();

    for (auto Item : Items)
    {
        if (!Item)
            continue;

        auto& ItemEntry = Item->GetItemEntry();
        FFortItemEntry DropItemEntry(ItemEntry.GetItemDefinition(), ItemEntry.GetCount(), ItemEntry.GetLevel());
        DropItemEntry.GetLoadedAmmo() = ItemEntry.GetLoadedAmmo();
        DropItemEntry.GetDurability() = ItemEntry.GetDurability();

        if (auto Pickup = AFortPickup::CreateFromData(Pawn->GetWorld(), DropItemEntry, Location, FRotator{ 0.0f, 0.0f, 0.0f }, PlayerController, AFortPickupAthena::StaticClass(), nullptr, EFortPickupSourceTypeFlag::AI))
            Pickup->TossPickup(Location, Pawn, 0, true);
    }
}

void AFortPlayerController::DropAllItems(const UFortItemDefinition* IgnoreItemDef, const UFortItemDefinition* AdditionalIgnoreItemDef, bool bIgnoreBuildingMaterials)
{
    static auto WorldResource = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WorldResource")));
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    TArray<TPair<FGuid, int32>> ItemsToDrop;

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
    {
        auto ItemDefinition = ItemEntry.GetItemDefinition();

        if (ItemDefinition && ItemDefinition != IgnoreItemDef && ItemDefinition != AdditionalIgnoreItemDef && (!bIgnoreBuildingMaterials || ItemDefinition->GetItemType() != WorldResource))
            ItemsToDrop.Add({ ItemEntry.GetItemGuid(), ItemEntry.GetCount() });
    }

    for (auto& ItemToDrop : ItemsToDrop)
        ServerSpawnInventoryDrop_Implementation(ItemToDrop.Key, ItemToDrop.Value);
}

void AFortPlayerController::execDropAllItems(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerController, execDropAllItems, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:DropAllItems"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerController:DropAllItems"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerController:DropAllItems")));

    static auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:DropAllItems"));
    static auto bHasAdditionalIgnoreItemDef = Function->FindPropertyByName(FName(TEXT("AdditionalIgnoreItemDef"))) != nullptr;
    static auto bHasIgnoreBuildingMaterials = Function->FindPropertyByName(FName(TEXT("bIgnoreBuildingMaterials"))) != nullptr;
    static auto bHasSpawnPickups = Function->FindPropertyByName(FName(TEXT("bSpawnPickups"))) != nullptr;

    UFortItemDefinition* IgnoreItemDef = nullptr;
    UFortItemDefinition* AdditionalIgnoreItemDef = nullptr;
    uint32 bIgnoreBuildingMaterials = 0;
    uint32 bSpawnPickups = 0;

    Stack.StepCompiledIn(&IgnoreItemDef);

    if (bHasAdditionalIgnoreItemDef)
        Stack.StepCompiledIn(&AdditionalIgnoreItemDef);

    if (bHasIgnoreBuildingMaterials)
        Stack.StepCompiledIn(&bIgnoreBuildingMaterials);

    if (bHasSpawnPickups)
        Stack.StepCompiledIn(&bSpawnPickups);

    Stack.Code += !!Stack.Code;

    DropAllItems(IgnoreItemDef, AdditionalIgnoreItemDef, !!bIgnoreBuildingMaterials);
}

void AFortPlayerController::DropSpecificItem(const UFortItemDefinition* DropItemDef)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory || !DropItemDef)
        return;

    TArray<TPair<FGuid, int32>> ItemsToDrop;

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
    {
        if (ItemEntry.GetItemDefinition() == DropItemDef)
            ItemsToDrop.Add({ ItemEntry.GetItemGuid(), ItemEntry.GetCount() });
    }

    for (auto& ItemToDrop : ItemsToDrop)
        ServerSpawnInventoryDrop_Implementation(ItemToDrop.Key, ItemToDrop.Value);
}

void AFortPlayerController::execDropSpecificItem(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerController, execDropSpecificItem, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:DropSpecificItem"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerController:DropSpecificItem"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerController:DropSpecificItem")));

    static auto bHasSpawnPickups = ((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:DropSpecificItem")))->FindPropertyByName(FName(TEXT("bSpawnPickups"))) != nullptr;

    UFortItemDefinition* DropItemDef = nullptr;
    uint32 bSpawnPickups = 0;

    Stack.StepCompiledIn(&DropItemDef);

    if (bHasSpawnPickups)
        Stack.StepCompiledIn(&bSpawnPickups);

    Stack.Code += !!Stack.Code;

    DropSpecificItem(DropItemDef);
}

TMulticastDelegate<void, AFortPlayerController*, const FName&, const FName&>& FFortPlayerControllerEvents::GetOnPlayerStateChangedEvent()
{
    static auto OnPlayerStateChangedEvent = Anchor::Data<TMulticastDelegate<void, AFortPlayerController*, const FName&, const FName&>*>(TEXT("FFortPlayerControllerEvents::OnPlayerStateChangedEvent"))
        .Exec(TEXT("/Script/Engine.PlayerController:ClientGotoState"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerController"))
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerController"))
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *OnPlayerStateChangedEvent;
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

    for (int32 Index = 0; Index < Inventory.GetReplicatedEntries().Num(); Index++)
    {
        auto& ItemEntry = Inventory.GetReplicatedEntries()[Index];

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

            Inventory.GetReplicatedEntries().RemoveAt(Index);
            Inventory.MarkArrayDirty();
            WorldInventory->ForceNetUpdate();
        }

        WorldInventory->HandleInventoryLocalUpdate();

        return true;
    }

    return false;
}

void AFortPlayerController::RemoveItemFromQuickBars(UFortItemDefinition* ItemDefinition)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto GetItemType = Anchor::VirtualFunction<uint8 (*)(const UFortItemDefinition*)>(TEXT("UFortItemDefinition::GetItemType"))
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [rax+0x*]")
            .Get();

        static auto IsQuickbarItemType = Anchor::Function<bool (*)(uint8)>(TEXT("AFortPlayerController::RemoveItemFromQuickBars item type check"))
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [rax+0x*]")
            .Find("call 0x*")
            .Target()
            .Get();

        static auto InventoryOwner = Anchor::Offset(TEXT("AFortPlayerController::RemoveItemFromQuickBars inventory owner"))
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("lea rcx, [rsi+0x*]")
            .Disp();

        static auto FindItemInstancesFromDefinition = Anchor::VirtualFunction<int32 (*)(void*, UFortItemDefinition*, TArray<UFortItem*>*)>(TEXT("IFortInventoryOwnerInterface::FindItemInstancesFromDefinition"))
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [rax+0x*]", 2)
            .Get();

        static auto RemoveItemFromQuickBar = Anchor::VirtualFunction<void (*)(AFortPlayerController*, bool, UFortItem*, uint8, int32)>(TEXT("AFortPlayerController::RemoveItemFromQuickBars remove item"))
            .Exec(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [rax+0x*]", 3)
            .Get();

        if (!ItemDefinition || !InventoryOwner || !IsQuickbarItemType(GetItemType(ItemDefinition)))
            return;

        static auto ItemVariantHandleStruct = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ItemVariantHandle"));

        TArray<UFortItem*> Items;
        alignas(8) uint8 ItemVariantHandle[0x18] = {};
        *(UFortItemDefinition**)ItemVariantHandle = ItemDefinition;

        if (FindItemInstancesFromDefinition((uint8*)this + InventoryOwner, ItemVariantHandleStruct ? (UFortItemDefinition*)ItemVariantHandle : ItemDefinition, &Items) <= 0)
            return;

        for (auto Item : Items)
            RemoveItemFromQuickBar(this, true, Item, 3, -2);

        return;
    }

    static auto RemoveItemFromQuickBars = Anchor::Function<void (*)(AFortPlayerController*, UFortItemDefinition*)>(TEXT("AFortPlayerController::RemoveItemFromQuickBars"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:RemoveItemFromQuickBars"))
        .Get();

    RemoveItemFromQuickBars(this, ItemDefinition);
}

AFortQuickBars* AFortPlayerController::GetQuickBars()
{
    static auto QuickBars = GetClass()->FindPropertyByName(FName(TEXT("QuickBars")));

    return QuickBars ? *QuickBars->ContainerPtrToValuePtr<AFortQuickBars*>(this) : nullptr;
}

void AFortPlayerController::SetQuickBars(AFortQuickBars* InQuickBars)
{
    static auto QuickBars = GetClass()->FindPropertyByName(FName(TEXT("QuickBars")));

    *QuickBars->ContainerPtrToValuePtr<AFortQuickBars*>(this) = InQuickBars;
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

void AFortPlayerController::ServerCreateBuildingActor_Implementation(FBuildingClassData BuildingClassData, const TGameValue<FVector>& GameBuildLoc, const TGameValue<FRotator>& GameBuildRot, bool bMirrored)
{
    ServerCreateBuildingActorInternal(BuildingClassData, GameBuildLoc, GameBuildRot, bMirrored, 0.0f, false, false, false);
}

ABuildingSMActor* AFortPlayerController::ServerCreateBuildingActorInternal(FBuildingClassData BuildingClassData, const TGameValue<FVector>& GameBuildLoc, const TGameValue<FRotator>& GameBuildRot, bool bMirrored, float SyncKey, bool bForceOverrideInteractBuildCheck, bool bIgnoreCostCheck, bool bIgnoreBuildingValidityCheck)
{
    DETOUR(AFortPlayerController, ServerCreateBuildingActorInternal, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco")))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [*+0x*]", 2)
        .LogSlot(TEXT("AFortDecoTool::ServerCreateBuildingAndSpawnDeco_Implementation"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("movss [rsp+0x28], xmm0; mov byte ptr [rsp+0x20], 0x0; call 0x*; test rax, rax")
        .Find("call 0x*")
        .IsTarget("xor eax, eax; ret*")
        .Optional()
        .AtCall(), Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco")) && StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap")))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [*+0x*]", 2)
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap"))
        .Find("movss [rsp+0x28], xmm0; mov byte ptr [rsp+0x20], 0x0; call 0x*; test rax, rax")
        .Find("call 0x*")
        .IsTarget("xor eax, eax; ret*")
        .Optional()
        .AtCall());

    FVector BuildLoc = GameBuildLoc.Get();
    FRotator BuildRot = GameBuildRot.Get();

    static auto bHasBuildChecks = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [*+0x*]", 2)
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("mov *[rsp+0x40], *")
        .Or()
        .At(DETOUR_ORIGINAL(AFortDecoTool, ServerCreateBuildingAndSpawnDeco_Implementation))
        .Get();

    bForceOverrideInteractBuildCheck = bHasBuildChecks && bForceOverrideInteractBuildCheck;
    bIgnoreCostCheck = bHasBuildChecks && bIgnoreCostCheck;
    bIgnoreBuildingValidityCheck = bHasBuildChecks && bIgnoreBuildingValidityCheck;

    static auto UpgradeLevel = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingClassData")))->FindPropertyByName(FName(TEXT("UpgradeLevel")));
    int32 BuildingLevel = 0;

    memcpy(&BuildingLevel, UpgradeLevel->ContainerPtrToValuePtr<void>(&BuildingClassData), UpgradeLevel->GetSize());

    BuildingClassData.UpgradeLevel = BuildingLevel;

    if (!ValidateBuildingClassData(BuildingClassData) || !BuildingClassData.BuildingClass)
        return nullptr;

    auto BuildingClass = BuildingClassData.BuildingClass;

    if (!BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return nullptr;

    if (!bForceOverrideInteractBuildCheck && !CanPerformNativeAction(FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.InteractBuild")))))
        return nullptr;

    const bool bBuildingRestricted = IsBuildingRestricted(BuildingClassData, BuildLoc, BuildRot);
    TArray<ABuildingActor*> ExistingBuildings;
    auto MarkerOptionalAdjustment = EFortBuildPreviewMarkerOptionalAdjustment::None;
    bool bCanAfford = false;
    auto QueryResults = EFortStructuralGridQueryResults::CanAdd;

    if (!bBuildingRestricted)
    {
        bCanAfford = bIgnoreCostCheck || CanAffordToPlaceBuildableClass(BuildingClassData);

        const bool bHasRequiredBuildingLevel = HasRequiredBuildingLevel(BuildingClassData);
        const FVector OldBuildLoc = BuildLoc;
        const FRotator OldBuildRot = BuildRot;

        QueryResults = CanPlaceBuildableClassInStructuralGrid(BuildingClass, BuildLoc, BuildRot, bMirrored, &ExistingBuildings, MarkerOptionalAdjustment);

        if (!BuildLoc.Equals(OldBuildLoc) || !BuildRot.Equals(OldBuildRot))
        {
            UE_LOG(LogFort, Warning, TEXT("ServerCreateBuildingActor_Implementation() yielded different build location after calling CanPlaceBuildableClassInStructuralGrid() - X=%3.3f Y=%3.3f Z=%3.3f P=%f Y=%f R=%f  to  X=%3.3f Y=%3.3f Z=%3.3f P=%f Y=%f R=%f"), OldBuildLoc.X, OldBuildLoc.Y, OldBuildLoc.Z, OldBuildRot.Pitch, OldBuildRot.Yaw, OldBuildRot.Roll, BuildLoc.X, BuildLoc.Y, BuildLoc.Z, BuildRot.Pitch, BuildRot.Yaw, BuildRot.Roll);
        }

        if (bCanAfford && bHasRequiredBuildingLevel && QueryResults == EFortStructuralGridQueryResults::CanAdd)
        {
            for (auto ExistingBuilding : ExistingBuildings)
            {
                if (ExistingBuilding && ExistingBuilding->GetDestroyOnPlayerBuildingPlacement())
                    ExistingBuilding->Die(FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Effect.InstantDeath.StructuralSupport"))), nullptr, nullptr, nullptr);
            }

            static auto bDeferConstruction = Anchor::Exists()
                .String(TEXT("Failed to remove item %s during pay building costs, item duplicated!"))
                .Caller()
                .FindCall(Anchor::Address()
                    .Native(TEXT("/Script/Engine.GameplayStatics:FinishSpawningActor"))
                    .Get())
                .Get();

            const FTransform SpawnTransform(BuildRot, BuildLoc, FVector(1.0f, 1.0f, 1.0f));
            ABuildingSMActor* BuildingActor = nullptr;

            if (bDeferConstruction)
            {
                BuildingActor = (ABuildingSMActor*)UGameplayStatics::BeginDeferredActorSpawnFromClass(this, BuildingClass->IsChildOf(ABuildingActor::StaticClass()) ? BuildingClass : nullptr, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, nullptr);
            }
            else
            {
                FActorSpawnParameters SpawnParameters;
                SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                BuildingActor = (ABuildingSMActor*)GetWorld()->SpawnActor(BuildingClass->IsChildOf(ABuildingActor::StaticClass()) ? BuildingClass : nullptr, &BuildLoc, &BuildRot, SpawnParameters);
            }

            if (!BuildingActor)
                return nullptr;

            BuildingActor->SetCurrentBuildingLevel(BuildingClassData.UpgradeLevel);
            BuildingActor->SetMirrored(bMirrored);
            BuildingActor->InitializeBuildingActor(EFortBuildingInitializationReason::Spawned, GetWorldPlayerId(), nullptr, nullptr, false);
            BuildingActor->PostInitializeSpawnedBuildingActor(EFortBuildingInitializationReason::Spawned);

            if (bDeferConstruction)
                UGameplayStatics::FinishSpawningActor(BuildingActor, SpawnTransform);

            UFortAnalytics::FireEvent_BuildingAction(this, TEXT("Create"), BuildingActor, bIgnoreCostCheck ? 0 : PayBuildableClassPlacementCost(BuildingClassData));

            if (auto GameMode = Cast<AFortGameMode>(GetWorld()->GetAuthGameMode()))
                GameMode->OnPlayerCreatedBuilding(this, BuildingActor);

            if (auto MyFortPawn = GetMyFortPawn())
            {
                UFortAIFunctionLibrary::MakeNoiseEventAtLocation(MyFortPawn, 1000.0f, BuildingActor->GetActorLocation());

                MyFortPawn->OnBlueprintPlace();

                if (auto BuildingTool = Cast<AFortWeap_BuildingTool>(MyFortPawn->GetCurrentWeapon()))
                    BuildingTool->PlayWeaponFireFX(false);
            }

            return BuildingActor;
        }
    }

    static auto BuildingIsRestrictedText = Anchor::Data<const FText*>(TEXT("BuildingIsRestrictedText"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [0x*]; lea rcx, *; call 0x*")
        .Target()
        .Get();

    static auto UnableToAfford = Anchor::Data<const FText*>(TEXT("UnableToAfford"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .TryCall("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; lea rdx, *; mov r8d, *; call 0x*")
        .Find("lea rdx, [0x*]")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("movzx *, byte ptr [rax+0x*]; call 0x*; mov rcx, rax; call 0x*; mov r8d, *; lea rdx, *; mov rcx, rax; call 0x*")
        .Find("lea rdx, [0x*]")
        .Target()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("movzx r8d, byte ptr [rax+0x*]; call 0x*; lea rdx, [0x*]")
        .Find("lea rdx, [0x*]")
        .Target()
        .Get();

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

    return nullptr;
}

void AFortPlayerController::ServerCreateBuildingActor_Implementation(const FCreateBuildingActorData& CreateBuildingData)
{
    auto GameState = Cast<AFortGameStateZone>(GetWorld()->GetGameState());

    if (!GameState)
        return;

    ServerCreateBuildingActor_Implementation(CreateBuildingData.GetBuildingClassData(GameState), CreateBuildingData.GetBuildLoc(), CreateBuildingData.GetBuildRot(), CreateBuildingData.GetMirrored());
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

void AFortPlayerController::ServerEditBuildingActor_Implementation(ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, uint8 RotationIterations, bool bMirrored)
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

    static auto NoEditPermissionText = Anchor::Data<const FText*>(TEXT("NoEditPermissionText"))
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("lea rdx, [0x*]")
        .Target()
        .Or()
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("mov rax, [0x*]; mov [rsp+0x*], rax; mov rax, [0x*]")
        .Target()
        .Or()
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("mov rax, [0x*]; mov [*], rax; mov rax, [0x*]")
        .Target()
        .Get();

    static auto BuildingLocked = Anchor::Data<const FText*>(TEXT("BuildingLocked"))
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("lea rdx, [0x*]", 2)
        .Target()
        .Or()
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("mov rax, [0x*]; mov [rsp+0x*], rax; mov rax, [0x*]", 2)
        .Target()
        .Or()
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("mov rax, [0x*]; mov [*], rax; mov rax, [0x*]", 2)
        .Target()
        .Get();

    static auto UnableToAffordRepair = Anchor::Data<const FText*>(TEXT("UnableToAffordRepair"))
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("lea rdx, [0x*]", 3)
        .Target()
        .Or()
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("mov rax, [0x*]; mov [rsp+0x*], rax; mov rax, [0x*]", 3)
        .Target()
        .Or()
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("mov rax, [0x*]; mov [*], rax; mov rax, [0x*]", 3)
        .Target()
        .Get();

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

    static auto bStartRepairSound = GetClass()->FindPropertyByName(FName(TEXT("StartRepairSound"))) != nullptr;

    if (auto StartRepairSound = bStartRepairSound ? GetStartRepairSound().LoadSynchronous() : nullptr)
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
    static auto CanPerformNativeAction = Anchor::VirtualFunction<bool (*)(const AFortPlayerController*, const FGameplayTag&)>(TEXT("AFortPlayerController::CanPerformNativeAction"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("call 0x*; mov rcx, *; lea rdx, [rax+0x*]; call [*]")
        .Find("call [*]")
        .SlotOrLastImm(this, "add *, 0x*")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("call 0x*; mov *, 0x*; mov rcx, *; lea rdx, [*+rax*1]; call [*]")
        .Find("call [*]")
        .SlotOrLastImm(this, "add *, 0x*")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("mov *, [rax+0x*]; call 0x*; mov *, 0x*; mov rcx, *; lea rdx, [*+rax*1]; call r*")
        .Get();

    return CanPerformNativeAction(this, ActionTag);
}

bool AFortPlayerController::IsBuildingRestricted(const FBuildingClassData& BuildingClassData, const FVector& BuildLoc, const FRotator& BuildRot) const
{
    static auto IsBuildingRestricted = Anchor::VirtualFunction<bool (*)(const AFortPlayerController*, const FBuildingClassData&, const TGameValue<FVector>&, const TGameValue<FRotator>&)>(TEXT("AFortPlayerController::IsBuildingRestricted"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("*, 0xffffffff")
        .Find("call [*+0x*]")
        .Get();

    return IsBuildingRestricted(this, BuildingClassData, BuildLoc, BuildRot);
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
    static auto HasRequiredBuildingLevel = Anchor::VirtualFunction<bool (*)(const AFortPlayerController*, int32, EFortResourceType)>(TEXT("AFortPlayerController::HasRequiredBuildingLevel"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("*, 0xffffffff")
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AFortPlayerController::IsBuildingRestricted"))
        .SlotShift(this, sizeof(void*))
        .Get();

    return HasRequiredBuildingLevel(this, BuildingLevel, ResourceType);
}

bool AFortPlayerController::CanAffordToPlaceBuildableClass(FBuildingClassData BuildingClassData) const
{
    static auto CanAffordToPlaceBuildableClass = Anchor::VirtualFunction<bool (*)(const AFortPlayerController*, FBuildingClassData)>(TEXT("AFortPlayerController::CanAffordToPlaceBuildableClass"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Call()
        .Find("call [*+0x*]; mov [rsp+0x*], al")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("mov r8, [*]; lea rdx, [rbp-0x*]; mov rcx, *; movdqa [rbp-0x*], xmm6; call [r8+0x*]")
        .Find("call [r8+0x*]")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("mov r8, [rcx+0x*]; mov rcx, *; call r8")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea rdx, [0x*]; lea rcx, *; call 0x*")
        .Find("call [*+0x*]")
        .Get();

    return CanAffordToPlaceBuildableClass(this, BuildingClassData);
}

EFortStructuralGridQueryResults AFortPlayerController::CanPlaceBuildableClassInStructuralGrid(UClass* BuildingClass, FVector& BuildLoc, FRotator& BuildRot, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings, EFortBuildPreviewMarkerOptionalAdjustment& MarkerOptionalAdjustment)
{
    static auto CanPlaceBuildableClassInStructuralGrid = Anchor::VirtualFunction<EFortStructuralGridQueryResults (*)(AFortPlayerController*, UClass*, TGameValue<FVector>&, TGameValue<FRotator>&, bool, TArray<ABuildingActor*>*)>(TEXT("AFortPlayerController::CanPlaceBuildableClassInStructuralGrid"))
        .String(TEXT("Failed to find intersection point!"))
        .Start()
        .Find("call [r10+0x*]")
        .Or()
        .String(TEXT("Failed to find intersection point!"))
        .Start()
        .Find("mov r10, [rax+0x*]; mov al, [*]; mov [rsp+0x28], rcx")
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("*, 0xffffffff")
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AFortPlayerController::IsBuildingRestricted"))
        .SlotShift(this, 10 * sizeof(void*))
        .Get();

    static auto bMarkerOptionalAdjustment = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EFortBuildPreviewMarkerOptionalAdjustment")) != nullptr;

    TGameValue<FVector> GameBuildLoc(BuildLoc);
    TGameValue<FRotator> GameBuildRot(BuildRot);

    const auto QueryResults = bMarkerOptionalAdjustment
        ? CanPlaceBuildableClassInStructuralGrid.As<EFortStructuralGridQueryResults (*)(AFortPlayerController*, UClass*, TGameValue<FVector>&, TGameValue<FRotator>&, bool, TArray<ABuildingActor*>*, EFortBuildPreviewMarkerOptionalAdjustment&)>()(this, BuildingClass, GameBuildLoc, GameBuildRot, bMirrored, ExistingBuildings, MarkerOptionalAdjustment)
        : CanPlaceBuildableClassInStructuralGrid(this, BuildingClass, GameBuildLoc, GameBuildRot, bMirrored, ExistingBuildings);

    BuildLoc = GameBuildLoc.Get();
    BuildRot = GameBuildRot.Get();

    return QueryResults;
}

int32 AFortPlayerController::PayBuildableClassPlacementCost(FBuildingClassData BuildingClassData)
{
    static auto PayBuildableClassPlacementCost = Anchor::VirtualFunction<int32 (*)(AFortPlayerController*, FBuildingClassData)>(TEXT("AFortPlayerController::PayBuildableClassPlacementCost"))
        .String(TEXT("Failed to remove item %s during pay building costs, item duplicated!"))
        .VirtualLoading(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
        .Or()
        .String(TEXT("Failed to remove item %s during pay building costs, item duplicated!"))
        .Target()
        .InVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"), TEXT("AFortPlayerController::PayBuildableClassPlacementCost"))
        .Get();

    return PayBuildableClassPlacementCost(this, BuildingClassData);
}

FText AFortPlayerController::GetErrorMessageFromStructuralQuery(EFortStructuralGridQueryResults QueryResults) const
{
    static auto GetErrorMessageFromStructuralQuery = Anchor::VirtualFunction<FText* (*)(const AFortPlayerController*, FText*, EFortStructuralGridQueryResults)>(TEXT("AFortPlayerController::GetErrorMessageFromStructuralQuery"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("mov r8d, *; *; call [*+0x*]")
        .Find("call [*+0x*]")
        .Get();

    if (!GetErrorMessageFromStructuralQuery)
        return FText();

    FText Result(NoInit);

    GetErrorMessageFromStructuralQuery(this, &Result, QueryResults);

    return Result;
}

bool AFortPlayerController::HasPermissionToEditWorld(ABuildingActor* BuildingActor, const FVector& Location, const FRotator& Rotation) const
{
    static auto HasPermissionToEditWorld = Anchor::VirtualFunction<bool (*)(const AFortPlayerController*, ABuildingActor*, const TGameValue<FVector>&, const TGameValue<FRotator>&)>(TEXT("AFortPlayerController::HasPermissionToEditWorld"))
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("call [*+0x*]", 2)
        .Get();

    return HasPermissionToEditWorld(this, BuildingActor, Location, Rotation);
}

bool AFortPlayerController::CanAffordToRepair(ABuildingSMActor* BuildingActorToRepair) const
{
    static auto CanAffordToRepair = Anchor::VirtualFunction<bool (*)(const AFortPlayerController*, ABuildingSMActor*)>(TEXT("AFortPlayerController::CanAffordToRepair"))
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("call [*+0x*]; test al, al; jz *; mov rdx, *; mov rcx, *; call 0x*")
        .Get();

    return CanAffordToRepair(this, BuildingActorToRepair);
}

int32 AFortPlayerController::PayBuildingRepairCost(ABuildingSMActor* BuildingActorToRepair)
{
    static auto PayBuildingRepairCost = Anchor::VirtualFunction<int32 (*)(AFortPlayerController*, ABuildingSMActor*)>(TEXT("AFortPlayerController::PayBuildingRepairCost"))
        .String(TEXT("Failed to remove item %s during repair, item duplicated!"))
        .Start()
        .Log(TEXT("AFortPlayerController::PayBuildingRepairCost"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
        .Get();

    return PayBuildingRepairCost(this, BuildingActorToRepair);
}

int32 AFortPlayerController::GetWorldPlayerId() const
{
    auto PlayerState = Cast<AFortPlayerState>(GetPlayerState());

    return PlayerState ? PlayerState->GetWorldPlayerId() : -1;
}

void AFortPlayerController::ClientSendMessage(const FText& MESSAGE)
{
    static auto ClientSendMessage = Anchor::Function<void (*)(AFortPlayerController*, const FText&, USoundBase*)>(TEXT("AFortPlayerController::ClientSendMessage"))
        .Event(TEXT("ClientSendMessage"))
        .Get();

    ClientSendMessage(this, MESSAGE, nullptr);
}

void AFortPlayerController::ClientExecuteInventoryItem(const FGuid& ItemGuid, float Delay, bool bForceExecute, bool bActivateSlotAfterSettingFocused)
{
    static auto ClientExecuteInventoryItem = Anchor::Function<void (*)(AFortPlayerController*, const FGuid&, float, bool, bool)>(TEXT("AFortPlayerController::ClientExecuteInventoryItem"))
        .Event(TEXT("ClientExecuteInventoryItem"))
        .Get();

    ClientExecuteInventoryItem(this, ItemGuid, Delay, bForceExecute, bActivateSlotAfterSettingFocused);
}

void AFortPlayerController::TriggerUIFeedbackEvent(FName EventName)
{
    static auto TriggerUIFeedbackEvent = Anchor::Function<void (*)(AFortPlayerController*, FName)>(TEXT("AFortPlayerController::TriggerUIFeedbackEvent"))
        .Event(TEXT("ClientTriggerUIFeedbackEvent"))
        .Caller()
        .Get();

    TriggerUIFeedbackEvent(this, EventName);
}

AFortPlayerPawn* AFortPlayerController::GetMyFortPawn() const
{
    static auto MyFortPawn = GetClass()->FindPropertyByName(FName(TEXT("MyFortPawn")));

    return *MyFortPawn->ContainerPtrToValuePtr<AFortPlayerPawn*>(this);
}

UFortMcpProfile* AFortPlayerController::GetAthenaProfile() const
{
    static auto AthenaProfile = StaticClass()->FindPropertyByName(FName(TEXT("AthenaProfile")));

    return AthenaProfile ? *AthenaProfile->ContainerPtrToValuePtr<UFortMcpProfile*>(this) : nullptr;
}

FMulticastScriptDelegate& AFortPlayerController::GetOnMcpProfilesInitializedEvent()
{
    static auto OnMcpProfilesInitializedEvent = StaticClass()->FindPropertyByName(FName(TEXT("OnMcpProfilesInitializedEvent")));

    return *OnMcpProfilesInitializedEvent->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

bool AFortPlayerController::IsPlayerAlive()
{
    static auto IsPlayerAlive = Anchor::VirtualFunction<bool (*)(AFortPlayerController*)>(TEXT("AFortPlayerController::IsPlayerAlive"))
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("mov rax, [rdi]; mov rcx, rdi; call [rax+0x*]; test al, al")
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("mov rax, [rsi]; mov rcx, rsi; call [rax+0x*]; test al, al")
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("[SCM] ABuildingGameplayActorSpawnMachine::ServerOnAttemptInteract - Start Rebooting"))
        .Start()
        .Find("mov rax, [r14]; mov rcx, r14; call [rax+0x*]; test al, al")
        .Find("call [rax+0x*]")
        .Get();

    return IsPlayerAlive(this);
}

TAssetPtr<UObject>& AFortPlayerController::GetStartRepairSound()
{
    static auto StartRepairSound = GetClass()->FindPropertyByName(FName(TEXT("StartRepairSound")));

    return *StartRepairSound->ContainerPtrToValuePtr<TAssetPtr<UObject>>(this);
}

void AFortPlayerController::ResetQuickBarToPrimary()
{
    static auto ResetQuickBarToPrimary = Anchor::Function<void (*)(AFortPlayerController*, bool)>(TEXT("AFortPlayerController::ResetQuickBarToPrimary"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerControllerZone:ClientOnPawnDied"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AFortPlayerControllerZone::ClientOnPawnDied_Implementation"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Call(2)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerControllerZone:ClientOnPawnDied"))
        .Find("mov rbx, [rax+0x*]; call 0x*; mov rdx, rax; mov rcx, *; call rbx")
        .LogSlot(TEXT("AFortPlayerControllerZone::ClientOnPawnDied_Implementation"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"))
        .Call(2)
        .Get();

    ResetQuickBarToPrimary(this, false);
}

bool AFortPlayerController::ForceEquipValidWeapon()
{
    DETOUR(AFortPlayerController, ForceEquipValidWeapon, Anchor::Hook()
        .Event(TEXT("ClientEndAbilityWeapon"))
        .Caller()
        .Find("test rdx, rdx; jz *; mov rax, [*]; mov rcx, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Find("call *")
        .Or()
        .String(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon failed to cleanup WeaponSeatDefinition.VehicleGrantedWeaponItem properly. PawnValid:%d"))
        .Start()
        .Find("mov rax, [r14]; mov rcx, r14; call [rax+0x*]; jmp 0x*")
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon failed to cleanup WeaponSeatDefinition.VehicleGrantedWeaponItem properly. PawnValid:%d"))
        .Start()
        .Optional()
        .Find("test rdx, rdx; jz *; mov rax, [*]; mov rcx, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Find("call *")
        .AtCall(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena")));

    auto MyFortPawn = GetMyFortPawn();

    if (MyFortPawn && MyFortPawn->GetLastEquippedWeaponOrGadgetGUID())
    {
        MyFortPawn->ServerEquipLastWeaponOrGadget_Implementation(true);

        return true;
    }

    return MyFortPawn && MyFortPawn->EquipBestWeapon();
}

UStatManager* AFortPlayerController::GetStatManager() const
{
    static auto StatManager = GetClass()->FindPropertyByName(FName(TEXT("StatManager")));

    return *StatManager->ContainerPtrToValuePtr<UStatManager*>(this);
}

void AFortPlayerController::SetAirSpawnControlRotation(const FRotator& InAirSpawnControlRotation)
{
    static auto AirSpawnControlRotation = Anchor::Offset(TEXT("AFortPlayerController::AirSpawnControlRotation"))
        .String(TEXT("FortGameModeAthena::FinishRestartPlayer -- Found potential teleport spot. Retrying."))
        .Start()
        .Find("cmp [rax+0x*], *")
        .FirstOf("movups xmm*, [r*+0x*]", "movsd xmm*, qword ptr [r*+0x*]")
        .Disp();

    TGameLayout<FRotator>::Store((uint8*)this + AirSpawnControlRotation, InAirSpawnControlRotation);
}

AFortInventory* AFortPlayerController::GetViewTargetInventory() const
{
    static auto ViewTargetInventory = GetClass()->FindPropertyByName(FName(TEXT("ViewTargetInventory")));

    return *ViewTargetInventory->ContainerPtrToValuePtr<AFortInventory*>(this);
}
