#include "pch.h"
#include "FortKismetLibrary.h"
#include "Abilities/FortAbilitySet.h"
#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
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

static auto PickLootDrops = []
{
    auto Exec = Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops"));
    auto Call = Exec.Find("call 0x*; lea rcx, [rsp+0x20]");
    auto Address = (Call.Get() ? Call : Exec.Find("mov [*+0x20], *; call 0x*").Find("call 0x*")).Get();

    if (((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:PickLootDrops")))->FindPropertyByName(FName(TEXT("WorldContextObject"))))
    {
        return DetourCall(TEXT("UFortKismetLibrary::PickLootDrops"), Address, (void*)+[](UObject* WorldContextObject, TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier)
        {
            return UFortKismetLibrary::PickLootDrops(OutLootToDrop, TierGroupName, WorldLevel, ForcedLootTier);
        });
    }

    return DetourCall(TEXT("UFortKismetLibrary::PickLootDrops"), Address, (void*)&UFortKismetLibrary::PickLootDrops);
}();

static auto K2_SpawnPickupInWorld = []
{
    auto Exec = Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_SpawnPickupInWorld"));
    auto Call = Exec.Find("call 0x*; mov rbx, [rsp+*]");

    return DetourCall(TEXT("UFortKismetLibrary::K2_SpawnPickupInWorld"), (Call.Get() ? Call : Exec.Find("mov [*+0x20], *; movsd [*], xmm*; call 0x*").Find("call 0x*")).Get(), &UFortKismetLibrary::K2_SpawnPickupInWorld);
}();

static auto K2_RemoveItemFromPlayerByGuid = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayerByGuid")) ? Detour(TEXT("UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid"), Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayerByGuid")).Get(), &UFortKismetLibrary::K2_RemoveItemFromPlayerByGuid) : nullptr;

static auto GiveItemToInventoryOwner = []
{
    auto GameGiveItemToInventoryOwner = Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner")).Get();
    auto Call = Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerControllerGameplay:StartGhostMode")).Find("call 0x*");

    while (Call.Get() && Call.Target() != GameGiveItemToInventoryOwner)
        Call = Call.Find("call 0x*");

    return Call.Get() ? DetourCall(TEXT("UFortKismetLibrary::GiveItemToInventoryOwner"), Call.Get(), &UFortKismetLibrary::GiveItemToInventoryOwner) : nullptr;
}();

FFortAbilitySetHandle UFortKismetLibrary::EquipFortAbilitySet(TScriptInterface<IAbilitySystemInterface> AbilitySystemInterfaceActor, UFortAbilitySet* AbilitySet, UObject* OverrideSourceObject)
{
    static auto EquipFortAbilitySet = Scanner::Function<FFortAbilitySetHandle* (*)(FFortAbilitySetHandle*, TScriptInterface<IAbilitySystemInterface>*, UFortAbilitySet*, UObject*)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:EquipFortAbilitySet")).Find("mov r8, [rsp+0x*]").Call().Get(TEXT("UFortKismetLibrary::EquipFortAbilitySet")));

    FFortAbilitySetHandle Handle;

    EquipFortAbilitySet(&Handle, &AbilitySystemInterfaceActor, AbilitySet, OverrideSourceObject);

    return Handle;
}

static auto execGiveItemToInventoryOwner = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner"));

    if (!Function || !Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GiveItemToInventoryOwner")).Is("ret*"))
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked UFortKismetLibrary::execGiveItemToInventoryOwner, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&UFortKismetLibrary::execGiveItemToInventoryOwner);

    return true;
}();

void UFortKismetLibrary::GiveItemToInventoryOwner(TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive, bool bNotifyPlayer)
{
    if (auto PlayerController = Cast<AFortPlayerController>(InventoryOwner.GetObject()))
        K2_GiveItemToPlayer(PlayerController, ItemDefinition, NumberToGive);
}

void UFortKismetLibrary::execGiveItemToInventoryOwner(FFrame& Stack, void* const Z_Param__Result)
{
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

static auto execK2_RemoveFortItemFromPlayer = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveFortItemFromPlayer"));

    if (!Function || Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveFortItemFromPlayer")).Get() != Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayer")).Get())
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked UFortKismetLibrary::execK2_RemoveFortItemFromPlayer, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&UFortKismetLibrary::execK2_RemoveFortItemFromPlayer);

    return true;
}();

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

void UFortKismetLibrary::BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier)
{
    static auto BroadcastSoundAtLocation = Scanner::Function<void (*)(UObject*, USoundBase*, const FVector&, float, float)>(Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:BroadcastSoundAtLocation")).Get(TEXT("UFortKismetLibrary::BroadcastSoundAtLocation")));

    BroadcastSoundAtLocation(WorldContextObject, InSound, Location, VolumeMultiplier, PitchMultiplier);
}

bool UFortKismetLibrary::PickLootDrops(TArray<FFortItemEntry>* OutLootToDrop, FName TierGroupName, int32 WorldLevel, int32 ForcedLootTier)
{
    FName LootTierKey(NAME_None);
    int32 LootTier = -1;

    if (!UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, LootTier, TierGroupName, WorldLevel, FGameplayTagContainer(), ForcedLootTier))
        return false;

    return UFortLootPackage::PickLootDrops(*OutLootToDrop, WorldLevel, LootTierKey, FGameplayTagContainer(), false, false);
}

FActiveGameplayEffectHandle UFortKismetLibrary::ApplyGlobalEnvironmentGameplayEffectToActor(AActor* EffectTargetActor, TSubclassOf<UGameplayEffect> GameplayEffect, int32 GameplayEffectLevel, const FGameplayTagContainer& AdditionalContextTags)
{
    static auto ApplyGlobalEnvironmentGameplayEffectToActor = Scanner::Function<FActiveGameplayEffectHandle* (*)(FActiveGameplayEffectHandle*, AActor*, UClass*, int32, const FGameplayTagContainer&)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:ApplyGlobalEnvironmentGameplayEffectToActor")).Find("mov [rsp+0x20], rax; call 0x*").Find("call 0x*").Target(TEXT("UFortKismetLibrary::ApplyGlobalEnvironmentGameplayEffectToActor")));

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
    ApplyGlobalEnvironmentGameplayEffectToActor(&ActiveGameplayEffectHandle, EffectTargetActor, GameplayEffect, GameplayEffectLevel, AdditionalContextTags);

    return ActiveGameplayEffectHandle;
}

AFortPickup* UFortKismetLibrary::K2_SpawnPickupInWorld(UObject* WorldContextObject, UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, const FVector& Position, const FVector& Direction, int32 OverrideMaxStackCount, bool bToss)
{
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

    auto Pickup = AFortPickup::CreateFromData(World, ItemEntry, Position, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass());

    if (Pickup && (bToss || !bHasToss))
        Pickup->TossPickup(Position, nullptr, OverrideMaxStackCount, true);

    return Pickup;
}

bool UFortKismetLibrary::AreAthenaVehiclesEnabled()
{
    static auto AreAthenaVehiclesEnabled = Scanner::Function<bool (*)()>(Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:AreAthenaVehiclesEnabled")).Get(TEXT("UFortKismetLibrary::AreAthenaVehiclesEnabled")));

    return AreAthenaVehiclesEnabled();
}

void UFortKismetLibrary::GetCalendarEventInformation(UObject* WorldContextObject, const FString& EventName, bool& bIsEventActive, FTimespan& TimeSinceBegin, FTimespan& TimeUntilEnd, float& TimespanRatio)
{
    static auto GetCalendarEventInformation = Scanner::Function<void (*)(UObject*, const FString&, bool&, FTimespan&, FTimespan&, float&)>(Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GetCalendarEventInformation")).Get(TEXT("UFortKismetLibrary::GetCalendarEventInformation")));

    GetCalendarEventInformation(WorldContextObject, EventName, bIsEventActive, TimeSinceBegin, TimeUntilEnd, TimespanRatio);
}
