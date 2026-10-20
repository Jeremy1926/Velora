#include "pch.h"
#include "Building/BuildingContainer.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "FortKismetLibrary.h"
#include "FortniteGame.h"
#include "Engine/World.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Logging/LogMacros.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

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
    static auto IsAlreadySearched = [this]
    {
        auto OnRep_bAlreadySearched = Anchor::Slot(TEXT("ABuildingContainer::OnRep_bAlreadySearched"))
            .Exec(TEXT("/Script/FortniteGame.BuildingContainer:OnRep_bAlreadySearched"))
            .Find("jmp [*]")
            .Get();
        auto OnRep = Scanner::At(Scanner::Virtual<const void*>(this, OnRep_bAlreadySearched));

        return OnRep.Is("test byte ptr [rcx+0x*], 0x*") ? Scanner::Slot(TEXT("ABuildingContainer::IsAlreadySearched"), OnRep_bAlreadySearched - (int32_t)sizeof(void*)) : OnRep.Find("call [rax+0x*]").Slot(TEXT("ABuildingContainer::IsAlreadySearched"));
    }();

    return Scanner::Virtual<bool (*)(const ABuildingContainer*)>(this, IsAlreadySearched)(this);
}

void ABuildingContainer::BounceContainer()
{
    static auto BounceContainer = Anchor::Slot(TEXT("ABuildingContainer::BounceContainer"))
        .Exec(TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer"))
        .Find("jmp [*+0x*]")
        .Get();

    Scanner::Virtual<void (*)(ABuildingContainer*)>(this, BounceContainer)(this);
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
                if (auto WorldItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition()))
                {
                    GetHighestRarity() = FMath::Max(GetHighestRarity(), WorldItemDefinition->GetRarity());

                    static auto WeaponMelee = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponMelee")));
                    static auto WeaponRanged = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponRanged")));

                    if (WorldItemDefinition->GetItemType() == WeaponMelee || WorldItemDefinition->GetItemType() == WeaponRanged)
                    {
                        ItemEntry.SetDurability(WorldItemDefinition->GetMaxDurability(ItemEntry.GetLevel()) * GetLootedWeaponsDurabilityModifier());
                        ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
                    }

                    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(WorldItemDefinition))
                    {
                        if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
                            ItemEntry.SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry.GetLevel()));
                    }

                    if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, LootSpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), nullptr, EFortPickupSourceTypeFlag::Container))
                    {
                        Pickup->GetTossedFromContainer() = true;
                        Pickup->TossPickup(LootSpawnLocation, nullptr, 0, true);

                        bSpawnedLoot = true;

                        auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(WorldItemDefinition);

                        if (WeaponItemDefinition && AFortGameStateAthena::AmmoDroppedFromContainersForWeaponPickup())
                        {
                            static auto GameplayTagAssetInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayTags.GameplayTagAssetInterface"));

                            FGameplayTagContainer SourceTags;
                            ((IGameplayTagAssetInterface*)GetInterfaceAddress(GameplayTagAssetInterface))->GetOwnedGameplayTags(SourceTags);

                            UFortKismetLibrary::CreateTossAmmoPickupForWeaponItemDefinitionAtLocation(this, WeaponItemDefinition, SourceTags, LootSpawnLocation, EFortPickupSourceTypeFlag::Container, EFortPickupSpawnSource::Unset);
                        }
                    }
                }
                else
                {
                    UE_LOG(LogFortBuilding, Warning, TEXT("Attempted to spawn non-world item %s!"), *GetNameSafe(ItemEntry.GetItemDefinition()));
                }
            }
        }

        if (!GetDestroyed())
        {
            SetContainerSearched(Searcher);

            if (ShouldDestroyContainerOnSearch())
                SilentDie(false);
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
    static auto SetContainerSearched = Anchor::Slot(TEXT("ABuildingContainer::SetContainerSearched"))
        .Exec(TEXT("/Script/FortniteGame.BuildingContainer:OnRep_bAlreadySearched"))
        .Find("jmp [*]")
        .LogSlot(TEXT("ABuildingContainer::OnRep_bAlreadySearched"))
        .Entry(this)
        .Find("xor edx, edx")
        .Find("jmp [*+0x*]")
        .Get();

    Scanner::Virtual<void (*)(ABuildingContainer*, AFortPawn*)>(this, SetContainerSearched)(this, Searcher);
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

bool ABuildingContainer::ShouldDestroyContainerOnSearch() const
{
    static auto ShouldDestroyContainerOnSearch = Anchor::Function<bool (*)(const ABuildingContainer*)>(TEXT("ABuildingContainer::ShouldDestroyContainerOnSearch"))
        .String(TEXT("Container %s has an empty SearchedMesh in its tier mesh set, and is not destroyed on search."))
        .Last("test al, al")
        .Last("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Container %s (%s) has an empty SearchedMesh in its tier mesh set, and is not destroyed on search."))
        .Last("test al, al")
        .Last("call 0x*")
        .Target()
        .Get();

    return ShouldDestroyContainerOnSearch(this);
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
