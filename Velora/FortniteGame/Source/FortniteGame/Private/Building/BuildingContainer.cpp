#include "pch.h"
#include "Building/BuildingContainer.h"
#include "Athena/FortGameModeAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWorldItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "FortniteGame.h"
#include "Engine/World.h"
#include "Logging/LogMacros.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "hde/hde64.h"

UClass* ABuildingContainer::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingContainer"));

    return Class;
}

void ABuildingContainer::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && GetStartAlreadySearched_Athena())
        SpawnLoot(nullptr);
}

bool ABuildingContainer::IsAlreadySearched() const
{
    static auto IsAlreadySearched = []
    {
        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingContainer:OnRep_bAlreadySearched")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32 - (int32_t)sizeof(void*);

            Address += Length;
        }
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const ABuildingContainer*))(VTable + IsAlreadySearched))(this);
}

void ABuildingContainer::BounceContainer()
{
    static auto BounceContainer = FindFunctionChecked(FName(TEXT("BounceContainer")));

    ProcessEvent(BounceContainer, nullptr);
}

bool ABuildingContainer::SpawnLoot(AFortPawn* Searcher)
{
    if (!HasAuthority())
        return false;

    ForceNetUpdate();

    bool bSpawnedLoot = false;

    if (!IsAlreadySearched())
    {
        const FVector LootSpawnLocation = GetTransform().TransformPosition(GetLootSpawnLocation());
        auto& ContainerLootTierKey = GetContainerLootTierKey();

        if (ContainerLootTierKey == NAME_None)
        {
            FName SearchLootTierGroup = GetSearchLootTierGroup();

            if (auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode()))
                SearchLootTierGroup = GameMode->RedirectLootTierGroup(SearchLootTierGroup);

            int32 LootTier = -1;

            UFortLootTier::ChooseUnquotaedLootTierKey(ContainerLootTierKey, LootTier, SearchLootTierGroup, -1, GetStaticGameplayTags());
        }

        if (ContainerLootTierKey != NAME_None)
        {
            TArray<FFortItemEntry> LootToDrop;

            UFortLootPackage::PickLootDrops(LootToDrop, -1, ContainerLootTierKey, GetStaticGameplayTags(), true, false);

            GetHighestRarity() = EFortRarity::Handmade;

            for (auto& ItemEntry : LootToDrop)
            {
                if (auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.ItemDefinition))
                {
                    GetHighestRarity() = FMath::Max(GetHighestRarity(), WorldItemDefinition->GetRarity());

                    if (WorldItemDefinition->GetItemType() == EFortItemType::WeaponMelee || WorldItemDefinition->GetItemType() == EFortItemType::WeaponRanged)
                    {
                        ItemEntry.SetDurability(WorldItemDefinition->GetMaxDurability(ItemEntry.Level) * GetLootedWeaponsDurabilityModifier());
                        ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
                    }

                    if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, LootSpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass()))
                    {
                        Pickup->GetTossedFromContainer() = true;
                        Pickup->TossPickup(LootSpawnLocation, nullptr, 0, true);

                        bSpawnedLoot = true;
                    }
                }
                else
                {
                    UE_LOG(LogFortBuilding, Warning, TEXT("Attempted to spawn non-world item %s!"), *GetNameSafe(ItemEntry.ItemDefinition));
                }
            }
        }

        if (!GetDestroyed())
        {
            SetContainerSearched(Searcher);

            if (GetDestroyContainerOnSearch())
                SilentDie();
        }
    }

    auto& SearchBounceData = GetSearchBounceData();

    if (Searcher)
    {
        const FVector SearcherLocation = Searcher->GetActorLocation();
        const FVector ContainerLocation = GetActorLocation();

        SearchBounceData.BounceNormal = FVector(SearcherLocation.X - ContainerLocation.X, SearcherLocation.Y - ContainerLocation.Y, 0.0f);
        SearchBounceData.BounceNormal.Normalize();
    }

    ++SearchBounceData.SearchAnimationCount;
    BounceContainer();

    return bSpawnedLoot;
}

void ABuildingContainer::SetContainerSearched(AFortPawn* Searcher)
{
    static auto SetContainerSearched = []
    {
        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32 + 5 * (int32_t)sizeof(void*);

            Address += Length;
        }
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(ABuildingContainer*, AFortPawn*))(VTable + SetContainerSearched))(this, Searcher);
}

FName ABuildingContainer::GetSearchLootTierGroup() const
{
    static auto SearchLootTierGroup = GetClass()->FindPropertyByName(FName(TEXT("SearchLootTierGroup")));

    return *SearchLootTierGroup->ContainerPtrToValuePtr<FName>(this);
}

FName& ABuildingContainer::GetContainerLootTierKey()
{
    static auto ContainerLootTierKey = GetClass()->FindPropertyByName(FName(TEXT("ContainerLootTierKey")));

    return *ContainerLootTierKey->ContainerPtrToValuePtr<FName>(this);
}

FVector ABuildingContainer::GetLootSpawnLocation() const
{
    static auto LootSpawnLocation = GetClass()->FindPropertyByName(FName(TEXT("LootSpawnLocation")));

    return *LootSpawnLocation->ContainerPtrToValuePtr<FVector>(this);
}

EFortRarity& ABuildingContainer::GetHighestRarity()
{
    static auto HighestRarity = GetClass()->FindPropertyByName(FName(TEXT("HighestRarity")));

    return *HighestRarity->ContainerPtrToValuePtr<EFortRarity>(this);
}

bool ABuildingContainer::GetDestroyContainerOnSearch() const
{
    static auto bDestroyContainerOnSearch = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDestroyContainerOnSearch")));

    return bDestroyContainerOnSearch->GetPropertyValue_InContainer(this);
}

bool ABuildingContainer::GetStartAlreadySearched_Athena() const
{
    static auto bStartAlreadySearched_Athena = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bStartAlreadySearched_Athena")));

    return bStartAlreadySearched_Athena->GetPropertyValue_InContainer(this);
}

float ABuildingContainer::GetLootedWeaponsDurabilityModifier() const
{
    static auto LootedWeaponsDurabilityModifier = GetClass()->FindPropertyByName(FName(TEXT("LootedWeaponsDurabilityModifier")));

    return *LootedWeaponsDurabilityModifier->ContainerPtrToValuePtr<float>(this);
}

FFortSearchBounceData& ABuildingContainer::GetSearchBounceData()
{
    static auto SearchBounceData = GetClass()->FindPropertyByName(FName(TEXT("SearchBounceData")));

    return *SearchBounceData->ContainerPtrToValuePtr<FFortSearchBounceData>(this);
}
