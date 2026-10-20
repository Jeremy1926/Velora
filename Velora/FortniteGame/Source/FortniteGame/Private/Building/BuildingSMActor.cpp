#include "pch.h"
#include "Building/BuildingSMActor.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortPickup.h"
#include "Items/FortResourceItemDefinition.h"
#include "Player/FortPlayerController.h"
#include "Player/FortPlayerState.h"
#include "Pawns/FortPawn.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "FortGameData.h"
#include "FortniteGame.h"
#include "Logging/LogMacros.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static int32_t GetEditingPlayerSlot()
{
    static auto GetEditingPlayer = Anchor::Slot(TEXT("ABuildingSMActor::GetEditingPlayer"))
        .String(TEXT("EditHPBlocked"))
        .Start()
        .Find("call [rax+0x???]", 2)
        .Get();

    return GetEditingPlayer;
}

static int32_t CanBePlayerEditedSlot()
{
    static auto CanBePlayerEdited = Anchor::Slot(TEXT("ABuildingSMActor::CanBePlayerEdited"))
        .String(TEXT("EditHPBlocked"))
        .Start()
        .Find("call [rax+0x???]", 5)
        .Get();

    return CanBePlayerEdited;
}

UClass* ABuildingSMActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingSMActor"));

    return Class;
}

void ABuildingSMActor::SetMirrored(bool bIsMirrored)
{
    static auto SetMirrored = []
    {
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingSMActor"));
        auto ReplaceBuildingActor = Scanner::Virtual<const void*>(DefaultObject, CanBePlayerEditedSlot(), -2 * (int32_t)sizeof(void*));
        auto SetCurrentBuildingLevel = Anchor::Slot(TEXT("ABuildingActor::GetCurrentBuildingLevel"))
            .Exec(TEXT("/Script/FortniteGame.BuildingActor:GetCurrentBuildingLevel"))
            .Find("call [*+0x*]")
            .Get() + (int32_t)sizeof(void*);

        return Anchor::Slot(TEXT("ABuildingSMActor::SetMirrored"))
            .At(ReplaceBuildingActor)
            .Find(std::format("call [*+{:#x}]", SetCurrentBuildingLevel))
            .Find("call [*+0x*]")
            .Get();
    }();

    Scanner::Virtual<void (*)(ABuildingSMActor*, bool)>(this, SetMirrored)(this, bIsMirrored);
}

ABuildingSMActor* ABuildingSMActor::ReplaceBuildingActor(EBuildingReplacementType ReplacementType, UClass* NewBuildingClass, int32 BuildingLevel, int32 RotationIterations, bool bMirrored, AFortPlayerController* PlayerController)
{
    return Scanner::Virtual<ABuildingSMActor* (*)(ABuildingSMActor*, EBuildingReplacementType, UClass*, int32, int32, bool, AFortPlayerController*)>(this, CanBePlayerEditedSlot(), -2 * (int32)sizeof(void*))(this, ReplacementType, NewBuildingClass, BuildingLevel, RotationIterations, bMirrored, PlayerController);
}

bool ABuildingSMActor::CanBePlayerEdited(AFortPlayerController* PlayerController)
{
    return Scanner::Virtual<bool (*)(ABuildingSMActor*, AFortPlayerController*)>(this, CanBePlayerEditedSlot())(this, PlayerController);
}

AFortPlayerStateZone* ABuildingSMActor::GetEditingPlayer() const
{
    return Scanner::Virtual<AFortPlayerStateZone* (*)(const ABuildingSMActor*)>(this, GetEditingPlayerSlot())(this);
}

void ABuildingSMActor::SetEditingPlayer(AFortPlayerStateZone* NewEditingPlayer)
{
    Scanner::Virtual<void (*)(ABuildingSMActor*, AFortPlayerStateZone*)>(this, GetEditingPlayerSlot(), sizeof(void*))(this, NewEditingPlayer);
}

bool ABuildingSMActor::IsBeingEdited() const
{
    return GetEditingPlayer() != nullptr;
}

bool ABuildingSMActor::NeedsRepair() const
{
    static auto NeedsRepair = Anchor::Slot(TEXT("ABuildingSMActor::NeedsRepair"))
        .Event(TEXT("ServerRepairBuildingActor"))
        .Caller()
        .Find("call [*+0x*]")
        .Get();

    return Scanner::Virtual<bool (*)(const ABuildingSMActor*)>(this, NeedsRepair)(this);
}

void ABuildingSMActor::RepairBuilding(AFortPlayerController* RepairingController, int32 ResourcesSpent)
{
    static auto RepairBuilding = Anchor::Slot(TEXT("ABuildingSMActor::RepairBuilding"))
        .Exec(TEXT("/Script/FortniteGame.BuildingSMActor:RepairBuilding"))
        .Find("call [*+0x*]")
        .Get();

    Scanner::Virtual<void (*)(ABuildingSMActor*, AFortPlayerController*, int32)>(this, RepairBuilding)(this, RepairingController, ResourcesSpent);
}

UObject* ABuildingSMActor::GetEditModePatternData() const
{
    static auto EditModePatternData = GetClass()->FindPropertyByName(FName(TEXT("EditModePatternData")));

    return *EditModePatternData->ContainerPtrToValuePtr<UObject*>(this);
}

void ABuildingSMActor::AttemptSpawnResources(AFortPawn* InstigatorPawn, float Damage, bool bJustHitWeakspot)
{
    if (!InstigatorPawn || !HasAuthority())
        return;

    auto PlayerController = Cast<AFortPlayerController>(InstigatorPawn->GetController());

    if (!PlayerController)
        return;

    const EFortResourceType ResourceType = GetResourceType();
    auto ResourceItemDefinition = UFortGameData::Get()->GetResourceItemDefinition(ResourceType);
    int32 ResourceCount = 0;
    float ResourcesPerDamage = 0.0f;

    if (ResourceItemDefinition)
    {
        auto& MaxResourcesToSpawn = GetMaxResourcesToSpawn();

        if (MaxResourcesToSpawn < 0)
            MaxResourcesToSpawn = (int32)DetermineMaxResourcesToSpawn(true);

        FGameplayTagContainer ResourceTags;
        ResourceItemDefinition->GetOwnedGameplayTags(ResourceTags);

        ResourcesPerDamage = (float)MaxResourcesToSpawn / GetMaxHealth();

        float ResourcesEarned = ResourcesPerDamage * Damage;
        auto PlayerState = (AFortPlayerState*)PlayerController->GetPlayerState();

        if (auto AbilitySystemComponent = PlayerState ? PlayerState->GetAbilitySystemComponent() : nullptr)
        {
            static auto ResourceHarvesting = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerAttrSet")))->FindPropertyByName(FName(TEXT("ResourceHarvesting")));

            bool bSuccess = false;
            const float HarvestedResources = UAbilitySystemBlueprintLibrary::EvaluateAttributeValueWithTagsAndBase((UAbilitySystemComponent*)AbilitySystemComponent, FGameplayAttribute(ResourceHarvesting), FGameplayTagContainer(), ResourceTags, ResourcesEarned, bSuccess);

            if (bSuccess)
                ResourcesEarned = HarvestedResources;
        }

        auto& UndistributedResources = GetUndistributedResources();

        ResourcesEarned += UndistributedResources;
        ResourceCount = (int32)ResourcesEarned;
        UndistributedResources = ResourcesEarned - (float)ResourceCount;
    }

    bool bDestroyed = false;

    if (!HasHealthLeft())
    {
        bDestroyed = true;
        ResourceCount = FMath::Max(ResourceCount, (ResourceItemDefinition && ResourcesPerDamage != 0.0f) ? 1 : 0);

        const FName DestructionLootTierKey = GetDestructionLootTierKey();

        if (DestructionLootTierKey != NAME_None)
        {
            TArray<FFortItemEntry> LootToDrop;
            UFortLootPackage::PickLootDrops(LootToDrop, -1, DestructionLootTierKey, GetStaticGameplayTags(), true, false);

            for (int32 Index = 0; Index < LootToDrop.Num(); ++Index)
            {
                auto& LootDrop = LootToDrop[Index];

                if (Cast<UFortWorldItemDefinition>(LootDrop.GetItemDefinition()))
                {
                    const FVector SpawnLocation = GetRootComponent()->GetBounds().Origin + FMath::VRand() * 200.0f;

                    FFortItemEntry ItemEntry(LootDrop.GetItemDefinition(), LootDrop.GetCount(), 0);
                    ItemEntry.SetStateValue(EFortItemEntryState::DoNotShowSpawnParticles, 1);

                    if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, SpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, nullptr, nullptr, EFortPickupSourceTypeFlag::Destruction))
                    {
                        const FVector StartDirection = FMath::VRandCone(FVector(0.0f, 0.0f, 1.0f), 0.2617994f);
                        const float FlyTime = (float)(Pickup->GetFlyTime() + Index * 0.3);

                        Pickup->SetPickupTarget(InstigatorPawn, FlyTime, StartDirection, true);
                    }
                }
                else
                {
                    UE_LOG(LogFortBuilding, Error, TEXT("Loot tier %s dropped entry with no item data!"), *DestructionLootTierKey.ToString());
                }
            }
        }
    }

    if (ResourceCount > 0)
    {
        if (auto WorldInventory = PlayerController->GetWorldInventory())
        {
            auto& Inventory = WorldInventory->GetInventory();
            const int32 MaxStackSize = ResourceItemDefinition->GetMaxStackSize(InstigatorPawn->GetAbilitySystemComponent());
            FFortItemEntry* ResourceItemEntry = nullptr;

            for (auto& ItemEntry : Inventory.GetReplicatedEntries())
            {
                if (ItemEntry.GetItemDefinition() == ResourceItemDefinition)
                {
                    ResourceItemEntry = &ItemEntry;
                    break;
                }
            }

            if (ResourceItemEntry)
            {
                const int32 NumberToStack = FMath::Min(ResourceCount, MaxStackSize - ResourceItemEntry->GetCount());

                if (NumberToStack > 0)
                {
                    ResourceItemEntry->SetCount(ResourceItemEntry->GetCount() + NumberToStack);
                    Inventory.MarkItemDirty(*ResourceItemEntry);
                }
            }
            else
            {
                auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(ResourceItemDefinition, FMath::Min(ResourceCount, MaxStackSize), 0);

                ItemEntry->GetParentInventory() = WorldInventory;
                ItemEntry->GetIsReplicatedCopy() = true;

                if (ItemEntry->GetParentInventory().IsValid())
                    ItemEntry->GetParentInventory().Get()->SetItemRequiresUpdate(*ItemEntry);

                ItemEntry->GetIsDirty() = true;
            }

            WorldInventory->HandleInventoryLocalUpdate();
        }

        PlayerController->ClientReportDamagedResourceBuilding(this, ResourceType, ResourceCount, bDestroyed, bJustHitWeakspot);
    }
}

float ABuildingSMActor::DetermineMaxResourcesToSpawn(bool bRandomize)
{
    static auto DetermineMaxResourcesToSpawn = Anchor::Slot(TEXT("ABuildingSMActor::DetermineMaxResourcesToSpawn"))
        .String(TEXT("ABuildingSMActor::DetermineMaxResourcesToSpawn"))
        .Last("mov rcx, [0x*]")
        .Target()
        .Reference("lea rdx, *")
        .Start()
        .Log(TEXT("ABuildingSMActor::DetermineMaxResourcesToSpawn"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__BuildingSMActor"))
        .Get();

    return Scanner::Virtual<float (*)(ABuildingSMActor*, bool)>(this, DetermineMaxResourcesToSpawn)(this, bRandomize);
}

EFortResourceType ABuildingSMActor::GetResourceType() const
{
    static auto ResourceType = GetClass()->FindPropertyByName(FName(TEXT("ResourceType")));

    return *ResourceType->ContainerPtrToValuePtr<EFortResourceType>(this);
}

int32& ABuildingSMActor::GetMaxResourcesToSpawn()
{
    static auto MaxResourcesToSpawn = GetClass()->FindPropertyByName(FName(TEXT("MaxResourcesToSpawn")));

    return *MaxResourcesToSpawn->ContainerPtrToValuePtr<int32>(this);
}

float& ABuildingSMActor::GetUndistributedResources()
{
    return *(float*)(&GetMaxResourcesToSpawn() + 1);
}

FName ABuildingSMActor::GetDestructionLootTierKey() const
{
    static auto DestructionLootTierKey = GetClass()->FindPropertyByName(FName(TEXT("DestructionLootTierKey")));

    return *DestructionLootTierKey->ContainerPtrToValuePtr<FName>(this);
}
