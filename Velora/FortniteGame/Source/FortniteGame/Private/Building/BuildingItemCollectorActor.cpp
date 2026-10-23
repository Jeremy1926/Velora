#include "pch.h"
#include "Building/BuildingItemCollectorActor.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortGameStateAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortLootTier.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "Player/FortPlayerController.h"
#include "FortKismetLibrary.h"
#include "GameplayEffectTypes.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

struct FPendingOutput
{
    TWeakObjectPtr<ABuildingItemCollectorActor> ItemCollector;
    TWeakObjectPtr<AFortPlayerController> Player;
    TWeakObjectPtr<UFortWorldItemDefinition> Item;
    float GrantTime;
};

static TArray<FPendingOutput> PendingOutputs;


UScriptStruct* FColletorUnitInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ColletorUnitInfo"));
    static auto CollectorUnitInfo = Struct ? Struct : (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.CollectorUnitInfo"));

    return CollectorUnitInfo;
}

int32 FColletorUnitInfo::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

UFortWorldItemDefinition*& FColletorUnitInfo::GetInputItem()
{
    static auto InputItem = StaticStruct()->FindPropertyByName(FName(TEXT("InputItem")));

    return *InputItem->ContainerPtrToValuePtr<UFortWorldItemDefinition*>(this);
}

const FScalableFloat& FColletorUnitInfo::GetInputCount()
{
    static auto InputCount = StaticStruct()->FindPropertyByName(FName(TEXT("InputCount")));

    return *InputCount->ContainerPtrToValuePtr<FScalableFloat>(this);
}

bool FColletorUnitInfo::GetUseDefinedOutputItem()
{
    static auto bUseDefinedOutputItem = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bUseDefinedOutputItem")));

    return bUseDefinedOutputItem->GetPropertyValue_InContainer(this);
}

UFortWorldItemDefinition*& FColletorUnitInfo::GetOutputItem()
{
    static auto OutputItem = StaticStruct()->FindPropertyByName(FName(TEXT("OutputItem")));

    return *OutputItem->ContainerPtrToValuePtr<UFortWorldItemDefinition*>(this);
}

TArray<FFortItemEntry>& FColletorUnitInfo::GetOutputItemEntry()
{
    static auto OutputItemEntry = StaticStruct()->FindPropertyByName(FName(TEXT("OutputItemEntry")));

    return *OutputItemEntry->ContainerPtrToValuePtr<TArray<FFortItemEntry>>(this);
}

FName FColletorUnitInfo::GetOverrideOutputItemLootTierGroupName()
{
    static auto OverrideOutputItemLootTierGroupName = StaticStruct()->FindPropertyByName(FName(TEXT("OverrideOutputItemLootTierGroupName")));

    return OverrideOutputItemLootTierGroupName->GetPropertyValue_InContainer<FName>(this);
}

UClass* ABuildingItemCollectorActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingItemCollectorActor"));

    return Class;
}

void ABuildingItemCollectorActor::BeginPlay()
{
    DETOUR(ABuildingItemCollectorActor, BeginPlay, Anchor::Hook()
        .If(FColletorUnitInfo::StaticStruct())
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Log(TEXT("ENGINE_ReceiveBeginPlay"))
        .Reference("mov ?dx, *")
        .Start()
        .Log(TEXT("AActor::ReceiveBeginPlay"))
        .Caller()
        .Log(TEXT("AActor::BeginPlay"))
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .LogSlot(TEXT("AActor::BeginPlay"))
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingItemCollectorActor")));

    Original();

    AthenaPostApplyPlaylistDataTables();
}

void ABuildingItemCollectorActor::AthenaPostApplyPlaylistDataTables()
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;
    auto VendingMachineRarityCount = MapInfo ? MapInfo->GetVendingMachineRarityCount() : nullptr;

    if (!HasAuthority() || !VendingMachineRarityCount)
        return;

    auto& ItemCollections = GetItemCollections();

    if (!ItemCollections.Num() || ItemCollections[0].GetOutputItemEntry().Num())
        return;

    TArray<float> RarityWeights;
    float TotalWeight = 0.0f;

    for (int32 Level = 0; ItemCollections[0].GetInputCount().GetValueAtLevel((float)Level) > 0.0f; Level++)
    {
        FName LootTierKey(NAME_None);
        int32 LootTier = -1;

        if (!UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, LootTier, GetDefaultItemLootTierGroupName(), -1, FGameplayTagContainer(), Level))
            break;

        RarityWeights.Add(VendingMachineRarityCount->GetValueAtLevel((float)Level));
        TotalWeight += RarityWeights[Level];
    }

    float RandomWeight = FMath::FRand() * TotalWeight;
    int32 StartingGoalLevel = 0;

    while (StartingGoalLevel < RarityWeights.Num() - 1 && RandomWeight > RarityWeights[StartingGoalLevel])
        RandomWeight -= RarityWeights[StartingGoalLevel++];

    GetStartingGoalLevel() = StartingGoalLevel;

    for (auto& ItemCollection : ItemCollections)
    {
        if (ItemCollection.GetUseDefinedOutputItem())
            continue;

        const FName OverrideTierGroupName = ItemCollection.GetOverrideOutputItemLootTierGroupName();
        auto& OutputItemEntry = ItemCollection.GetOutputItemEntry();

        UFortKismetLibrary::PickLootDrops(&OutputItemEntry, OverrideTierGroupName != NAME_None ? OverrideTierGroupName : GetDefaultItemLootTierGroupName(), -1, StartingGoalLevel);

        for (auto& ItemEntry : OutputItemEntry)
        {
            auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition());

            if (!WorldItemDefinition)
                continue;

            static auto WeaponMelee = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponMelee")));
            static auto WeaponRanged = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponRanged")));

            if (WorldItemDefinition->GetItemType() == WeaponMelee || WorldItemDefinition->GetItemType() == WeaponRanged)
            {
                ItemEntry.SetDurability(WorldItemDefinition->GetMaxDurability(ItemEntry.GetLevel()));
                ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
            }

            if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(WorldItemDefinition))
            {
                if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
                    ItemEntry.SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry.GetLevel()));
            }
        }

        ItemCollection.GetOutputItem() = OutputItemEntry.Num() ? Cast<UFortWorldItemDefinition>(OutputItemEntry[0].GetItemDefinition()) : nullptr;
    }

    ForceNetUpdate();
}

void ABuildingItemCollectorActor::CheckDepositGoalReached(AFortPlayerController* Player, UFortWorldItemDefinition* Item)
{
    DETOUR(ABuildingItemCollectorActor, CheckDepositGoalReached, Anchor::Hook()
        .If(FColletorUnitInfo::StaticStruct())
        .Native(TEXT("/Script/FortniteGame.BuildingItemCollectorActor:GetDepositGoal"))
        .Log(TEXT("ABuildingItemCollectorActor::GetDepositGoal"))
        .Reference("call 0x*; xor *, *; mov *, eax; test *, *")
        .Start()
        .Or()
        .If(FColletorUnitInfo::StaticStruct())
        .Native(TEXT("/Script/FortniteGame.BuildingItemCollectorActor:GetDepositGoal"))
        .Reference("call 0x*; mov r8, *; mov rdx, *; mov rcx, *; mov ebx, eax")
        .Start()
        .Or()
        .If(FColletorUnitInfo::StaticStruct())
        .Native(TEXT("/Script/FortniteGame.BuildingItemCollectorActor:GetDepositGoal"))
        .Reference("call 0x*; mov *, eax; test rsi, rsi; jz 0x*; mov r9b, 0x1")
        .Start()
        .Inline());

    auto& ControllingPlayer = GetControllingPlayer();
    auto LastControllingPlayer = ControllingPlayer;

    ControllingPlayer = nullptr;

    Original(Player, Item);

    if (!Player || ControllingPlayer != Player)
    {
        ControllingPlayer = LastControllingPlayer;
        return;
    }

    if (GetDespensingDelayTime() > 0.0f)
    {
        PendingOutputs.Add(FPendingOutput{ this, Player, Item, GetWorld()->GetTimeSeconds() + GetDespensingDelayTime() });
        return;
    }

    GrantOutput(Item);
}

void ABuildingItemCollectorActor::GrantOutput(UFortWorldItemDefinition* Item)
{
    const FVector LootSpawnLocation = GetTransform().TransformPosition(GetLootSpawnLocation());

    for (auto& ItemCollection : GetItemCollections())
    {
        if (ItemCollection.GetInputItem() != Item)
            continue;

        for (auto& ItemEntry : ItemCollection.GetOutputItemEntry())
        {
            if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, LootSpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass()))
            {
                Pickup->GetTossedFromContainer() = true;
                Pickup->TossPickup(LootSpawnLocation, nullptr, 0, true);

                PickupSpawned_Bind();
            }
        }

        return;
    }
}

void ABuildingItemCollectorActor::TickPendingOutputs()
{
    for (int32 Index = PendingOutputs.Num() - 1; Index >= 0; Index--)
    {
        auto ItemCollector = PendingOutputs[Index].ItemCollector.Get();

        if (ItemCollector && ItemCollector->GetWorld()->GetTimeSeconds() < PendingOutputs[Index].GrantTime)
            continue;

        auto Player = PendingOutputs[Index].Player.Get();
        auto Item = PendingOutputs[Index].Item.Get();

        PendingOutputs.RemoveAt(Index);

        if (ItemCollector && Player)
            ItemCollector->GrantOutput(Item);
    }
}

void ABuildingItemCollectorActor::PickupSpawned_Bind()
{
    static auto B_Athena_VendingMachine_C = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C"));
    static auto SpawnedItemCount = B_Athena_VendingMachine_C ? B_Athena_VendingMachine_C->FindPropertyByName(FName(TEXT("SpawnedItemCount"))) : nullptr;
    static auto MaxItemsToSpawnCached = B_Athena_VendingMachine_C ? B_Athena_VendingMachine_C->FindPropertyByName(FName(TEXT("MaxItemsToSpawnCached"))) : nullptr;

    if (!SpawnedItemCount || !MaxItemsToSpawnCached || !IsA(B_Athena_VendingMachine_C))
        return;

    auto& ItemCount = *SpawnedItemCount->ContainerPtrToValuePtr<int32>(this);
    const int32 MaxItemsToSpawn = *MaxItemsToSpawnCached->ContainerPtrToValuePtr<int32>(this);

    ItemCount++;

    if (MaxItemsToSpawn > 0 && ItemCount >= MaxItemsToSpawn)
    {
        DoVendDeath();
        Destroy();
    }
}

void ABuildingItemCollectorActor::DoVendDeath()
{
    static auto DoVendDeath = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Game/Athena/Items/Gameplay/VendingMachine/B_Athena_VendingMachine.B_Athena_VendingMachine_C:DoVendDeath"));

    if (DoVendDeath)
        CallRemoteFunction(DoVendDeath, nullptr, nullptr, nullptr);
}

FName ABuildingItemCollectorActor::GetDefaultItemLootTierGroupName() const
{
    static auto DefaultItemLootTierGroupName = GetClass()->FindPropertyByName(FName(TEXT("DefaultItemLootTierGroupName")));

    return DefaultItemLootTierGroupName->GetPropertyValue_InContainer<FName>(this);
}

TArray<FColletorUnitInfo>& ABuildingItemCollectorActor::GetItemCollections()
{
    static auto ItemCollections = GetClass()->FindPropertyByName(FName(TEXT("ItemCollections")));

    return *ItemCollections->ContainerPtrToValuePtr<TArray<FColletorUnitInfo>>(this);
}

AFortPlayerController*& ABuildingItemCollectorActor::GetControllingPlayer()
{
    static auto ControllingPlayer = GetClass()->FindPropertyByName(FName(TEXT("ControllingPlayer")));

    return *ControllingPlayer->ContainerPtrToValuePtr<AFortPlayerController*>(this);
}

float ABuildingItemCollectorActor::GetDespensingDelayTime() const
{
    static auto DespensingDelayTime = GetClass()->FindPropertyByName(FName(TEXT("DespensingDelayTime")));

    return *DespensingDelayTime->ContainerPtrToValuePtr<float>(this);
}

FVector ABuildingItemCollectorActor::GetLootSpawnLocation() const
{
    static auto LootSpawnLocation = GetClass()->FindPropertyByName(FName(TEXT("LootSpawnLocation")));

    return *LootSpawnLocation->ContainerPtrToValuePtr<FVector>(this);
}

int32& ABuildingItemCollectorActor::GetStartingGoalLevel()
{
    static auto StartingGoalLevel = GetClass()->FindPropertyByName(FName(TEXT("StartingGoalLevel")));

    return *StartingGoalLevel->ContainerPtrToValuePtr<int32>(this);
}
