#include "pch.h"
#include "FortKismetLibrary.h"
#include "FortGameModeZone.h"
#include "Building/BuildingItemCollectorActor.h"
#include "Building/BuildingItemWeaponUpgradeActor.h"
#include "Abilities/FortAbilitySet.h"
#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

FFortAbilitySetHandle UFortKismetLibrary::EquipFortAbilitySet(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterfaceActor, UFortAbilitySet* AbilitySet, UObject* OverrideSourceObject)
{
    static auto EquipFortAbilitySet = Anchor::Function<FFortAbilitySetHandle* (*)(FFortAbilitySetHandle*, TScriptInterface<IAbilitySystemInterface>*, UFortAbilitySet*, UObject*)>(TEXT("UFortKismetLibrary::EquipFortAbilitySet"))
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:EquipFortAbilitySet"))
        .Find("mov r8, [rsp+0x*]")
        .Call()
        .Get();

    FFortAbilitySetHandle Handle;

    EquipFortAbilitySet(&Handle, &AbilitySystemInterfaceActor, AbilitySet, OverrideSourceObject);

    return Handle;
}

void UFortKismetLibrary::GiveItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer)
{
    DETOUR(UFortKismetLibrary, GiveItemToInventoryOwner, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerControllerGameplay:StartGhostMode"))
        .FindCall(Anchor::Address()
            .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner"))
            .Get())
        .Optional()
        .AtCall());

    if (auto PlayerController = Cast<AFortPlayerController>(InventoryOwner.GetObject()))
        K2_GiveItemToPlayer(PlayerController, ItemDefinition, NumberToGive);
}

void UFortKismetLibrary::execGiveItemToInventoryOwner(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortKismetLibrary, execGiveItemToInventoryOwner, Anchor::Hook()
        .After(DETOUR_ORIGINAL(UFortKismetLibrary, GiveItemToInventoryOwner))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner"))
        .Is("ret*")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner")));

    static auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner"));
    static auto bHasItemLevel = Function->FindPropertyByName(FName(TEXT("ItemLevel"))) != nullptr;
    static auto bHasPickupInstigatorHandle = Function->FindPropertyByName(FName(TEXT("PickupInstigatorHandle"))) != nullptr;

    TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
    UFortWorldItemDefinition* ItemDefinition = nullptr;
    int32 NumberToGive = 0;
    uint32 bNotifyPlayer = 0;
    int32 ItemLevel = 0;
    int32 PickupInstigatorHandle = 0;

    Stack.StepCompiledIn(&InventoryOwner);
    Stack.StepCompiledIn(&ItemDefinition);
    Stack.StepCompiledIn(&NumberToGive);
    Stack.StepCompiledIn(&bNotifyPlayer);

    if (bHasItemLevel)
        Stack.StepCompiledIn(&ItemLevel);

    if (bHasPickupInstigatorHandle)
        Stack.StepCompiledIn(&PickupInstigatorHandle);

    Stack.Code += !!Stack.Code;

    GiveItemToInventoryOwner(InventoryOwner, ItemDefinition, NumberToGive, !!bNotifyPlayer);
}

void UFortKismetLibrary::K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive)
{
    if (!PlayerController || !ItemDefinition)
        return;

    auto WorldInventory = PlayerController->GetWorldInventory();

    if (!WorldInventory)
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(ItemDefinition, NumberToGive, ItemDefinition->GetMinLevel());
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;

    if (ItemEntry->GetParentInventory().IsValid())
        ItemEntry->GetParentInventory().Get()->SetItemRequiresUpdate(*ItemEntry);

    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    if (!PlayerController->CanServerTryAddToQuickBar())
        return;

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
            PlayerController->TryAddToQuickBar(Item);
    }
}

int32 UFortKismetLibrary::K2_RemoveItemFromPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 AmountToRemove, bool bForceRemoval)
{
    DETOUR(UFortKismetLibrary, K2_RemoveItemFromPlayer, Anchor::Hook()
        .If(FColletorUnitInfo::StaticStruct())
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayer"))
        .Inline(), Anchor::Hook()
        .After(DETOUR_ORIGINAL(ABuildingItemWeaponUpgradeActor, GrantOutput))
        .If(ABuildingItemWeaponUpgradeActor::StaticClass())
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayer"))
        .Find("jnz 0x*")
        .Target()
        .Log(TEXT("UFortKismetLibrary::K2_RemoveItemFromPlayer"))
        .AtCallsIn(Anchor::Address()
            .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
            .Start()
            .Get()));

    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory || !ItemDefinition)
        return 0;

    auto& ReplicatedEntries = WorldInventory->GetInventory().GetReplicatedEntries();
    int32 AmountRemoved = 0;

    for (int32 Index = ReplicatedEntries.Num() - 1; Index >= 0 && (AmountToRemove < 0 || AmountRemoved < AmountToRemove); Index--)
    {
        if (ReplicatedEntries[Index].GetItemDefinition() != ItemDefinition)
            continue;

        const FGuid ItemGuid = ReplicatedEntries[Index].GetItemGuid();
        const int32 Count = AmountToRemove < 0 ? ReplicatedEntries[Index].GetCount() : FMath::Min(ReplicatedEntries[Index].GetCount(), AmountToRemove - AmountRemoved);

        if (PlayerController->RemoveInventoryItem(ItemGuid, Count, bForceRemoval))
            AmountRemoved += Count;
    }

    return AmountRemoved;
}

int32 UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid(AFortPlayerController* PlayerController, const FGuid& ItemGuid, int32 AmountToRemove, bool bForceRemoval)
{
    DETOUR(UFortKismetLibrary, K2_RemoveItemFromPlayerByGuid, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayerByGuid")))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayerByGuid"))
        .Inline());

    auto WorldInventory = PlayerController && ItemGuid.IsValid() ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return 0;

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
    {
        if (ItemEntry.GetItemGuid() != ItemGuid)
            continue;

        const int32 Count = FMath::Min(ItemEntry.GetCount(), AmountToRemove);

        return PlayerController->RemoveInventoryItem(ItemGuid, Count, bForceRemoval) ? Count : 0;
    }

    return 0;
}

int32 UFortKismetLibrary::K2_RemoveFortItemFromPlayer(AFortPlayerController* PlayerController, UFortItem* Item, int32 AmountToRemove, bool bForceRemoval)
{
    auto WorldInventory = PlayerController && Item ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return 0;

    const FGuid ItemGuid = Item->GetItemGuid();

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
    {
        if (ItemEntry.GetItemGuid() != ItemGuid)
            continue;

        const int32 Count = AmountToRemove < 0 ? ItemEntry.GetCount() : FMath::Min(ItemEntry.GetCount(), AmountToRemove);

        return PlayerController->RemoveInventoryItem(ItemGuid, Count, bForceRemoval) ? Count : 0;
    }

    return 0;
}

void UFortKismetLibrary::execK2_RemoveFortItemFromPlayer(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortKismetLibrary, execK2_RemoveFortItemFromPlayer, Anchor::Hook()
        .If(Anchor::Address().Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveFortItemFromPlayer")).Get() == Anchor::Address().Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayer")).Get())
        .InExec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveFortItemFromPlayer")));

    AFortPlayerController* PlayerController = nullptr;
    UFortItem* Item = nullptr;
    int32 AmountToRemove = 0;
    uint32 bForceRemoval = 0;

    Stack.StepCompiledIn(&PlayerController);
    Stack.StepCompiledIn(&Item);
    Stack.StepCompiledIn(&AmountToRemove);
    Stack.StepCompiledIn(&bForceRemoval);
    Stack.Code += !!Stack.Code;

    *(int32*)Z_Param__Result = K2_RemoveFortItemFromPlayer(PlayerController, Item, AmountToRemove, !!bForceRemoval);
}

AFortPlayerController* UFortKismetLibrary::GetFortPlayerControllerFromActor(AActor* Actor)
{
    static auto GetFortPlayerControllerFromActor = Anchor::Function<AFortPlayerController* (*)(AActor*)>(TEXT("UFortKismetLibrary::GetFortPlayerControllerFromActor"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GetFortPlayerControllerFromActor"))
        .Get();

    return GetFortPlayerControllerFromActor(Actor);
}

float UFortKismetLibrary::GetServerWorldTimeSeconds(const UObject* WorldContextObject)
{
    static auto GetServerWorldTimeSeconds = Anchor::Function<float (*)(const UObject*)>(TEXT("UFortKismetLibrary::GetServerWorldTimeSeconds"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GetServerWorldTimeSeconds"))
        .Get();

    return GetServerWorldTimeSeconds(WorldContextObject);
}

void UFortKismetLibrary::BroadcastAbilityImpactAtLocation(UObject* WorldContextObject, USoundBase* InSound, UObject* Instigator, const FVector& Position, float Radius, FName Tag, const bool bSubtractLocalNoise)
{
    static auto BroadcastAbilityImpactAtLocation = Anchor::Function<void (*)(UObject*, USoundBase*, UObject*, const FVector&, float, FName, bool)>(TEXT("UFortKismetLibrary::BroadcastAbilityImpactAtLocation"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:BroadcastAbilityImpactAtLocation"))
        .Get();

    BroadcastAbilityImpactAtLocation(WorldContextObject, InSound, Instigator, Position, Radius, Tag, bSubtractLocalNoise);
}

void UFortKismetLibrary::BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier)
{
    static auto BroadcastSoundAtLocation = Anchor::Function<void (*)(UObject*, USoundBase*, const FVector&, float, float)>(TEXT("UFortKismetLibrary::BroadcastSoundAtLocation"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:BroadcastSoundAtLocation"))
        .Get();

    BroadcastSoundAtLocation(WorldContextObject, InSound, Location, VolumeMultiplier, PitchMultiplier);
}

bool UFortKismetLibrary::PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier)
{
    DETOUR(UFortKismetLibrary, PickLootDrops, Anchor::Hook(((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops")))->FindPropertyByName(FName(TEXT("WorldContextObject"))) ? (void*)+[](UObject* WorldContextObject, TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier)
        {
            return UFortKismetLibrary::PickLootDrops(OutLootToDrop, TierGroupName, WorldLevel, ForcedLootTier);
        } : (void*)&UFortKismetLibrary::PickLootDrops)
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops"))
        .Find("call 0x*; lea rcx, [rsp+0x20]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops"))
        .Find("mov [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .AtCall());

    FName LootTierKey(NAME_None);
    int32 LootTier = -1;

    if (!UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, LootTier, TierGroupName, WorldLevel, FGameplayTagContainer(), ForcedLootTier))
        return false;

    return UFortLootPackage::PickLootDrops(*OutLootToDrop, WorldLevel, LootTierKey, FGameplayTagContainer(), false, false);
}

FActiveGameplayEffectHandle UFortKismetLibrary::ApplyGlobalEnvironmentGameplayEffectToActor(AActor* EffectTargetActor, TSubclassOf<UGameplayEffect> GameplayEffect, int32 GameplayEffectLevel, const FGameplayTagContainer& AdditionalContextTags)
{
    static auto ApplyGlobalEnvironmentGameplayEffectToActor = Anchor::Function<FActiveGameplayEffectHandle* (*)(FActiveGameplayEffectHandle*, AActor*, UClass*, int32, const FGameplayTagContainer&)>(TEXT("UFortKismetLibrary::ApplyGlobalEnvironmentGameplayEffectToActor"))
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:ApplyGlobalEnvironmentGameplayEffectToActor"))
        .Find("mov [rsp+0x20], rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
    ApplyGlobalEnvironmentGameplayEffectToActor(&ActiveGameplayEffectHandle, EffectTargetActor, GameplayEffect, GameplayEffectLevel, AdditionalContextTags);

    return ActiveGameplayEffectHandle;
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss)
{
    DETOUR(UFortKismetLibrary, K2_SpawnPickupInWorld, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld"))
        .Find("call 0x*; mov rbx, [rsp+*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld"))
        .Find("mov [*+0x20], *; movsd [*], xmm*; call 0x*")
        .Find("call 0x*")
        .AtCall());

    return K2_SpawnPickupInWorldWithClass(WorldContextObject, ItemDefinition, AFortPickupAthena::StaticClass(), NumberToSpawn, Position, Direction, OverrideMaxStackCount, bToss);
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorldWithClass(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, TSubclassOf<AFortPickup> PickupClass, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss)
{
    DETOUR(UFortKismetLibrary, K2_SpawnPickupInWorldWithClass, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorldWithClass")))
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorldWithClass"))
        .Find("mov [*+0x20], *; movsd [*], xmm*; call 0x*")
        .Find("call 0x*")
        .AtCall());

    static auto bHasToss = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld")))->FindPropertyByName(FName(TEXT("bToss"))) != nullptr;

    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    if (!World || !ItemDefinition || NumberToSpawn <= 0)
        return nullptr;

    FFortItemEntry ItemEntry(ItemDefinition, NumberToSpawn, ItemDefinition->GetMinLevel());

    static auto WeaponMelee = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponMelee")));
    static auto WeaponRanged = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponRanged")));

    if (ItemDefinition->GetItemType() == WeaponMelee || ItemDefinition->GetItemType() == WeaponRanged)
    {
        ItemEntry.SetDurability(ItemDefinition->GetMaxDurability(ItemEntry.GetLevel()));
        ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
    }

    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
    {
        if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
            ItemEntry.SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry.GetLevel()));
    }

    auto Pickup = AFortPickup::CreateFromData(World, ItemEntry, Position, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, PickupClass);

    if (Pickup && (bToss || !bHasToss))
        Pickup->TossPickup(Position, nullptr, OverrideMaxStackCount, true);

    return Pickup;
}

TArray<AFortPickup*> UFortKismetLibrary::K2_SpawnPickupInWorldWithLootTier(UObject* WorldContextObject, FName LootTierName, const FVector& Position, int32 OverrideMaxStackCount, bool bToss, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source)
{
    DETOUR(UFortKismetLibrary, K2_SpawnPickupInWorldWithLootTier, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorldWithLootTier")))
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorldWithLootTier"))
        .Find("mov [rsp+0x20], *; call 0x*")
        .Find("call 0x*")
        .IsTarget("xor eax, eax; mov [rcx], rax; mov [rcx+0x8], rax; mov rax, rcx; ret")
        .Optional()
        .AtCall());

    TArray<AFortPickup*> Pickups;
    TArray<FFortItemEntry> LootToDrop;
    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    if (!World || !PickLootDrops(&LootToDrop, LootTierName, -1, -1))
        return Pickups;

    for (auto& ItemEntry : LootToDrop)
    {
        if (!Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition()))
            continue;

        auto Pickup = AFortPickup::CreateFromData(World, ItemEntry, Position, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), nullptr, SourceType);

        if (!Pickup)
            continue;

        if (bToss)
            Pickup->TossPickup(Position, nullptr, OverrideMaxStackCount, true, true, SourceType, Source);

        Pickups.Add(Pickup);
    }

    return Pickups;
}

FSpawnItemVariantParams::FSpawnItemVariantParams()
{
    FMemory::Memzero(this, sizeof(FSpawnItemVariantParams));

    StaticStruct()->InitializeStruct(this);
}

FSpawnItemVariantParams::~FSpawnItemVariantParams()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FSpawnItemVariantParams::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.SpawnItemVariantParams"));

    return Struct;
}

UFortWorldItemDefinition* FSpawnItemVariantParams::GetWorldItemDefinition() const
{
    static auto WorldItemDefinition = StaticStruct()->FindPropertyByName(FName(TEXT("WorldItemDefinition")));

    return *WorldItemDefinition->ContainerPtrToValuePtr<UFortWorldItemDefinition*>(this);
}

int32 FSpawnItemVariantParams::GetNumberToSpawn() const
{
    static auto NumberToSpawn = StaticStruct()->FindPropertyByName(FName(TEXT("NumberToSpawn")));

    return *NumberToSpawn->ContainerPtrToValuePtr<int32>(this);
}

const FVector& FSpawnItemVariantParams::GetPosition() const
{
    static auto Position = StaticStruct()->FindPropertyByName(FName(TEXT("Position")));

    return *Position->ContainerPtrToValuePtr<FVector>(this);
}

const FVector& FSpawnItemVariantParams::GetDirection() const
{
    static auto Direction = StaticStruct()->FindPropertyByName(FName(TEXT("Direction")));

    return *Direction->ContainerPtrToValuePtr<FVector>(this);
}

int32 FSpawnItemVariantParams::GetOverrideMaxStackCount() const
{
    static auto OverrideMaxStackCount = StaticStruct()->FindPropertyByName(FName(TEXT("OverrideMaxStackCount")));

    return *OverrideMaxStackCount->ContainerPtrToValuePtr<int32>(this);
}

bool FSpawnItemVariantParams::GetToss() const
{
    static auto bToss = StaticStruct()->FindPropertyByName(FName(TEXT("bToss")));

    return *bToss->ContainerPtrToValuePtr<bool>(this);
}

bool FSpawnItemVariantParams::GetRandomRotation() const
{
    static auto bRandomRotation = StaticStruct()->FindPropertyByName(FName(TEXT("bRandomRotation")));

    return *bRandomRotation->ContainerPtrToValuePtr<bool>(this);
}

bool FSpawnItemVariantParams::GetBlockedFromAutoPickup() const
{
    static auto bBlockedFromAutoPickup = StaticStruct()->FindPropertyByName(FName(TEXT("bBlockedFromAutoPickup")));

    return *bBlockedFromAutoPickup->ContainerPtrToValuePtr<bool>(this);
}

int32 FSpawnItemVariantParams::GetPickupInstigatorHandle() const
{
    static auto PickupInstigatorHandle = StaticStruct()->FindPropertyByName(FName(TEXT("PickupInstigatorHandle")));

    return *PickupInstigatorHandle->ContainerPtrToValuePtr<int32>(this);
}

EFortPickupSourceTypeFlag FSpawnItemVariantParams::GetSourceType() const
{
    static auto SourceType = StaticStruct()->FindPropertyByName(FName(TEXT("SourceType")));

    return *SourceType->ContainerPtrToValuePtr<EFortPickupSourceTypeFlag>(this);
}

EFortPickupSpawnSource FSpawnItemVariantParams::GetSource() const
{
    static auto Source = StaticStruct()->FindPropertyByName(FName(TEXT("Source")));

    return *Source->ContainerPtrToValuePtr<EFortPickupSpawnSource>(this);
}

AFortPlayerController* FSpawnItemVariantParams::GetOptionalOwnerPC() const
{
    static auto OptionalOwnerPC = StaticStruct()->FindPropertyByName(FName(TEXT("OptionalOwnerPC")));

    return *OptionalOwnerPC->ContainerPtrToValuePtr<AFortPlayerController*>(this);
}

bool FSpawnItemVariantParams::GetPickupOnlyRelevantToOwner() const
{
    static auto bPickupOnlyRelevantToOwner = StaticStruct()->FindPropertyByName(FName(TEXT("bPickupOnlyRelevantToOwner")));

    return *bPickupOnlyRelevantToOwner->ContainerPtrToValuePtr<bool>(this);
}

AFortPickup* UFortKismetLibrary::SpawnItemVariantPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* WorldItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss, bool bRandomRotation, bool bBlockedFromAutoPickup, int32 PickupInstigatorHandle, EFortPickupSourceTypeFlag SourceType, EFortPickupSpawnSource Source, AFortPlayerController* OptionalOwnerPC, bool bPickupOnlyRelevantToOwner)
{
    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    if (!World || !WorldItemDefinition)
        return nullptr;

    FFortItemEntry ItemEntry(WorldItemDefinition, NumberToSpawn, 0);

    auto Pickup = AFortPickup::CreateFromData(World, ItemEntry, Position, Direction.ToOrientationRotator(), OptionalOwnerPC, nullptr, nullptr, SourceType, bRandomRotation, bPickupOnlyRelevantToOwner, Source);

    if (Pickup)
        Pickup->TossPickup(Position, nullptr, OverrideMaxStackCount, bToss, false, SourceType, Source);

    return Pickup;
}

void UFortKismetLibrary::execSpawnItemVariantPickupInWorld(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortKismetLibrary, execSpawnItemVariantPickupInWorld, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:SpawnItemVariantPickupInWorld"))
        .Find("movsd [*], xmm0; call 0x*")
        .Find("call 0x*")
        .IsTarget("xor eax, eax; ret")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:SpawnItemVariantPickupInWorld"))
        .Find("movsd [*], xmm0; call 0x*")
        .Find("call 0x*")
        .IsTarget("push rbx; sub rsp, 0x20; mov rcx, [rdx+0x18]; mov rbx, rdx; test rcx, rcx; jz 0x*; call 0x*; mov rcx, [rbx+0x8]")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortKismetLibrary:SpawnItemVariantPickupInWorld")));

    static auto Params = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:SpawnItemVariantPickupInWorld")))->FindPropertyByName(FName(TEXT("Params")));

    UObject* WorldContextObject = nullptr;

    Stack.StepCompiledIn(&WorldContextObject);

    if (Params)
    {
        FSpawnItemVariantParams SpawnParams;

        Stack.StepCompiledIn(&SpawnParams);
        Stack.Code += !!Stack.Code;

        *(AFortPickup**)Z_Param__Result = SpawnItemVariantPickupInWorld(WorldContextObject, SpawnParams.GetWorldItemDefinition(), SpawnParams.GetNumberToSpawn(), SpawnParams.GetPosition(), SpawnParams.GetDirection(), SpawnParams.GetOverrideMaxStackCount(), SpawnParams.GetToss(), SpawnParams.GetRandomRotation(), SpawnParams.GetBlockedFromAutoPickup(), SpawnParams.GetPickupInstigatorHandle(), SpawnParams.GetSourceType(), SpawnParams.GetSource(), SpawnParams.GetOptionalOwnerPC(), SpawnParams.GetPickupOnlyRelevantToOwner());

        return;
    }

    UFortWorldItemDefinition* WorldItemDefinition = nullptr;
    int32 NumberToSpawn = 0;
    FVector Position(ForceInit);
    FVector Direction(ForceInit);
    int32 OverrideMaxStackCount = 0;
    uint32 bToss = 0;
    uint32 bRandomRotation = 0;
    uint32 bBlockedFromAutoPickup = 0;
    int32 PickupInstigatorHandle = 0;
    EFortPickupSourceTypeFlag SourceType = EFortPickupSourceTypeFlag::Other;
    EFortPickupSpawnSource Source = EFortPickupSpawnSource::Unset;
    AFortPlayerController* OptionalOwnerPC = nullptr;
    uint32 bPickupOnlyRelevantToOwner = 0;

    Stack.StepCompiledIn(&WorldItemDefinition);
    Stack.StepCompiledIn(&NumberToSpawn);
    Stack.StepCompiledIn(&Position);
    Stack.StepCompiledIn(&Direction);
    Stack.StepCompiledIn(&OverrideMaxStackCount);
    Stack.StepCompiledIn(&bToss);
    Stack.StepCompiledIn(&bRandomRotation);
    Stack.StepCompiledIn(&bBlockedFromAutoPickup);
    Stack.StepCompiledIn(&PickupInstigatorHandle);
    Stack.StepCompiledIn(&SourceType);
    Stack.StepCompiledIn(&Source);
    Stack.StepCompiledIn(&OptionalOwnerPC);
    Stack.StepCompiledIn(&bPickupOnlyRelevantToOwner);
    Stack.Code += !!Stack.Code;

    *(AFortPickup**)Z_Param__Result = SpawnItemVariantPickupInWorld(WorldContextObject, WorldItemDefinition, NumberToSpawn, Position, Direction, OverrideMaxStackCount, !!bToss, !!bRandomRotation, !!bBlockedFromAutoPickup, PickupInstigatorHandle, SourceType, Source, OptionalOwnerPC, !!bPickupOnlyRelevantToOwner);
}

void UFortKismetLibrary::CreateTossAmmoPickupForWeaponItemDefinitionAtLocation(UObject* WorldContextObject, const UFortWeaponItemDefinition* WeaponItemDefinition, const FGameplayTagContainer& SourceTags, const FVector& Location, const EFortPickupSourceTypeFlag SourceTypeFlag, const EFortPickupSpawnSource SpawnSource)
{
    static auto CreateTossAmmoPickupForWeaponItemDefinitionAtLocation = Anchor::Function<void (*)(UObject*, const UFortWeaponItemDefinition*, const FGameplayTagContainer&, const FVector&, EFortPickupSourceTypeFlag, EFortPickupSpawnSource)>(TEXT("UFortKismetLibrary::CreateTossAmmoPickupForWeaponItemDefinitionAtLocation"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:CreateTossAmmoPickupForWeaponItemDefinitionAtLocation"))
        .Get();

    CreateTossAmmoPickupForWeaponItemDefinitionAtLocation(WorldContextObject, WeaponItemDefinition, SourceTags, Location, SourceTypeFlag, SpawnSource);
}

bool UFortKismetLibrary::AreAthenaVehiclesEnabled()
{
    static auto AreAthenaVehiclesEnabled = Anchor::Function<bool (*)()>(TEXT("UFortKismetLibrary::AreAthenaVehiclesEnabled"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:AreAthenaVehiclesEnabled"))
        .Get();

    return AreAthenaVehiclesEnabled();
}

bool UFortKismetLibrary::UpgradeAllWeaponsVerticalToRarity(AFortPlayerController* PlayerController, EFortRarity NewRarity)
{
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return false;

    auto WeaponUpgradeActor = ABuildingItemWeaponUpgradeActor::StaticClass()->GetDefaultObject<ABuildingItemWeaponUpgradeActor>();
    TArray<TPair<UFortWeaponItemDefinition*, UFortWeaponItemDefinition*>> Upgrades;

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
    {
        auto WeaponDefinition = Cast<UFortWeaponItemDefinition>(ItemEntry.GetItemDefinition());
        auto UpgradedWeaponDefinition = WeaponDefinition;

        while (UpgradedWeaponDefinition)
        {
            auto ItemRow = WeaponUpgradeActor->GetItemRow(UpgradedWeaponDefinition, EFortWeaponUpgradeDirection::Vertical);
            auto NextWeaponDefinition = ItemRow ? ItemRow->GetUpgradedWeaponDef() : nullptr;

            if (!NextWeaponDefinition || NextWeaponDefinition->GetRarity() > NewRarity)
                break;

            UpgradedWeaponDefinition = NextWeaponDefinition;
        }

        if (UpgradedWeaponDefinition != WeaponDefinition)
            Upgrades.Add({ WeaponDefinition, UpgradedWeaponDefinition });
    }

    for (auto& Upgrade : Upgrades)
    {
        K2_RemoveItemFromPlayer(PlayerController, Upgrade.Key, 1, false);
        WeaponUpgradeActor->GrantOutput(PlayerController, Upgrade.Value);
    }

    return Upgrades.Num() > 0;
}

void UFortKismetLibrary::execUpgradeAllWeaponsVerticalToRarity(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortKismetLibrary, execUpgradeAllWeaponsVerticalToRarity, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:UpgradeAllWeaponsVerticalToRarity"))
        .Is("xor al, al; ret")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortKismetLibrary:UpgradeAllWeaponsVerticalToRarity")));

    static auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:UpgradeAllWeaponsVerticalToRarity"));
    static auto bHasThrottle = Function->FindPropertyByName(FName(TEXT("bThrottle"))) != nullptr;

    AFortPlayerController* PlayerController = nullptr;
    EFortRarity NewRarity = {};
    uint32 bThrottle = 0;

    Stack.StepCompiledIn(&PlayerController);
    Stack.StepCompiledIn(&NewRarity);

    if (bHasThrottle)
        Stack.StepCompiledIn(&bThrottle);

    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = UpgradeAllWeaponsVerticalToRarity(PlayerController, NewRarity);
}

void UFortKismetLibrary::GetCalendarEventInformation(UObject* WorldContextObject, const FString& EventName, bool& bIsEventActive, FTimespan& TimeSinceBegin, FTimespan& TimeUntilEnd, float& TimespanRatio)
{
    static auto GetCalendarEventInformation = Anchor::Function<void (*)(UObject*, const FString&, bool&, FTimespan&, FTimespan&, float&)>(TEXT("UFortKismetLibrary::GetCalendarEventInformation"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GetCalendarEventInformation"))
        .Get();

    GetCalendarEventInformation(WorldContextObject, EventName, bIsEventActive, TimeSinceBegin, TimeUntilEnd, TimespanRatio);
}

AFortAIDirector* UFortKismetLibrary::GetAIDirector(const UObject* WorldContextObject)
{
    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    auto GameMode = World ? Cast<AFortGameModeZone>(World->GetAuthGameMode()) : nullptr;

    return GameMode ? GameMode->GetAIDirector() : nullptr;
}

void UFortKismetLibrary::execGetAIDirector(FFrame& Stack, void* const Z_Param__Result)
{
    UObject* WorldContextObject = nullptr;

    Stack.StepCompiledIn(&WorldContextObject);
    Stack.Code += !!Stack.Code;

    *(AFortAIDirector**)Z_Param__Result = GetAIDirector(WorldContextObject);
}
