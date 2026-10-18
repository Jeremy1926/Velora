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

UClass* ABuildingSMActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingSMActor"));

    return Class;
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

                if (Cast<UFortWorldItemDefinition>(LootDrop.ItemDefinition))
                {
                    const FVector SpawnLocation = GetRootComponent()->GetBounds().Origin + FMath::VRand() * 200.0f;

                    FFortItemEntry ItemEntry(LootDrop.ItemDefinition, LootDrop.Count, 0);
                    ItemEntry.SetStateValue(EFortItemEntryState::DoNotShowSpawnParticles, 1);

                    if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, SpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }))
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
        FFortItemEntry ItemEntry(ResourceItemDefinition, ResourceCount, 0);

        const FVector SpawnLocation = GetRootComponent()->GetBounds().Origin + FMath::VRand() * 200.0f;

        ItemEntry.SetStateValue(EFortItemEntryState::DoNotShowSpawnParticles, 1);

        if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, SpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }))
        {
            const FVector StartDirection = FMath::VRand();

            Pickup->SetPickupTarget(InstigatorPawn, Pickup->GetFlyTime(), StartDirection, true);

            PlayerController->ClientReportDamagedResourceBuilding(this, ResourceType, ResourceCount, bDestroyed, bJustHitWeakspot);
        }
    }
}

float ABuildingSMActor::DetermineMaxResourcesToSpawn(bool bRandomize)
{
    static auto DetermineMaxResourcesToSpawn = []
    {
        auto Function = Scanner::String(TEXT("ABuildingSMActor::DetermineMaxResourcesToSpawn")).Start().Get();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingSMActor"));
        auto Index = 0;

        while (VTable[Index] != Function)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    return (*(float (**)(ABuildingSMActor*, bool))(VTable + DetermineMaxResourcesToSpawn))(this, bRandomize);
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
