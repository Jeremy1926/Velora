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
#include "Pawns/FortPlayerPawn.h"
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
#include "Detour.h"

UClass* ABuildingContainer::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingContainer"));

    return Class;
}

void ABuildingContainer::BeginPlay()
{
    Super::BeginPlay();

    auto GameStateAthena = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (HasAuthority() && GetStartAlreadySearched_Athena() && (!DETOUR_ORIGINAL(AFortGameStateAthena, OnPlaylistLootTableAssetsDataLoadCompleted) || !GameStateAthena || GameStateAthena->GetPlaylistDataIsLoaded()))
        SpawnLoot(nullptr);
}

bool ABuildingContainer::IsAlreadySearched() const
{
    static auto IsAlreadySearched = Anchor::VirtualFunction<bool (*)(const ABuildingContainer*)>(TEXT("ABuildingContainer::IsAlreadySearched"))
        .Exec(TEXT("/Script/FortniteGame.BuildingContainer:OnRep_bAlreadySearched"))
        .Find("jmp [*]")
        .LogSlot(TEXT("ABuildingContainer::OnRep_bAlreadySearched"))
        .IsVirtual(this, "test byte ptr [rcx+0x*], 0x*")
        .SlotShift(this, -(int64_t)sizeof(void*))
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingContainer:OnRep_bAlreadySearched"))
        .Find("jmp [*]")
        .Entry(this)
        .Find("call [rax+0x*]")
        .Get();

    return IsAlreadySearched(this);
}

void ABuildingContainer::BounceContainer()
{
    static auto BounceContainer = Anchor::VirtualFunction<void (*)(ABuildingContainer*)>(TEXT("ABuildingContainer::BounceContainer"))
        .Exec(TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer"))
        .Find("jmp [*+0x*]")
        .Get();

    BounceContainer(this);
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
        auto ContainerLootTierKey = GetContainerLootTierKey();

        if (ContainerLootTierKey == NAME_None)
        {
            FName SearchLootTierGroup = GetSearchLootTierGroup();

            if (auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode()))
                SearchLootTierGroup = GameMode->RedirectLootTierGroup(SearchLootTierGroup);

            int32 LootTier = -1;

            UFortLootTier::ChooseUnquotaedLootTierKey(ContainerLootTierKey, LootTier, SearchLootTierGroup, -1, GetStaticGameplayTags());
            SetContainerLootTierKey(ContainerLootTierKey);
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

        FVector BounceNormal(SearcherLocation.X - ContainerLocation.X, SearcherLocation.Y - ContainerLocation.Y, 0.0f);
        BounceNormal.Normalize();

        SearchBounceData.SetBounceNormal(BounceNormal);
    }

    ++SearchBounceData.GetSearchAnimationCount();
    SearchBounceData.SetSearchingPawn(Cast<AFortPlayerPawn>(Searcher));
    BounceContainer();

    return bSpawnedLoot;
}

void ABuildingContainer::SetContainerSearched(AFortPawn* Searcher)
{
    static auto SetContainerSearched = Anchor::VirtualFunction<void (*)(ABuildingContainer*, AFortPawn*)>(TEXT("ABuildingContainer::SetContainerSearched"))
        .Exec(TEXT("/Script/FortniteGame.BuildingContainer:OnRep_bAlreadySearched"))
        .Find("jmp [*]")
        .LogSlot(TEXT("ABuildingContainer::OnRep_bAlreadySearched"))
        .Entry(this)
        .Find("xor edx, edx")
        .Find("jmp [*+0x*]")
        .Get();

    SetContainerSearched(this, Searcher);
}

FName ABuildingContainer::GetSearchLootTierGroup() const
{
    static auto SearchLootTierGroup = GetClass()->FindPropertyByName(FName(TEXT("SearchLootTierGroup")));

    return SearchLootTierGroup->GetPropertyValue_InContainer<FName>(this);
}

FName ABuildingContainer::GetContainerLootTierKey()
{
    static auto ContainerLootTierKey = GetClass()->FindPropertyByName(FName(TEXT("ContainerLootTierKey")));

    return ContainerLootTierKey->GetPropertyValue_InContainer<FName>(this);
}

void ABuildingContainer::SetContainerLootTierKey(FName InContainerLootTierKey)
{
    static auto ContainerLootTierKey = GetClass()->FindPropertyByName(FName(TEXT("ContainerLootTierKey")));

    ContainerLootTierKey->SetPropertyValue_InContainer<FName>(this, InContainerLootTierKey);
}

FVector ABuildingContainer::GetLootSpawnLocation() const
{
    static auto LootSpawnLocation = GetClass()->FindPropertyByName(FName(TEXT("LootSpawnLocation")));

    return LootSpawnLocation->GetPropertyValue_InContainer<FVector>(this);
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
        .Or()
        .String(TEXT("Container %s (%s) has an empty SearchedMesh in its tier mesh set, and is not destroyed on search."))
        .Start()
        .Find("call 0x*; test al, al")
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

UScriptStruct* FFortSearchBounceData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSearchBounceData"));

    return Struct;
}

void FFortSearchBounceData::SetBounceNormal(const FVector& InBounceNormal)
{
    static auto BounceNormal = StaticStruct() ? StaticStruct()->FindPropertyByName(FName(TEXT("BounceNormal"))) : nullptr;

    TGameLayout<FVector>::Store(BounceNormal ? BounceNormal->ContainerPtrToValuePtr<void>(this) : (void*)this, InBounceNormal);
}

uint32& FFortSearchBounceData::GetSearchAnimationCount()
{
    static auto SearchAnimationCount = StaticStruct() ? StaticStruct()->FindPropertyByName(FName(TEXT("SearchAnimationCount"))) : nullptr;

    return SearchAnimationCount ? *SearchAnimationCount->ContainerPtrToValuePtr<uint32>(this) : *(uint32*)((uint8*)this + TGameLayout<FVector>::Size());
}

void FFortSearchBounceData::SetSearchingPawn(AFortPlayerPawn* InSearchingPawn)
{
    static auto SearchingPawn = StaticStruct()->FindPropertyByName(FName(TEXT("SearchingPawn")));

    if (SearchingPawn)
        *SearchingPawn->ContainerPtrToValuePtr<AFortPlayerPawn*>(this) = InSearchingPawn;
}

FFortSearchBounceData& ABuildingContainer::GetSearchBounceData()
{
    static auto SearchBounceData = GetClass()->FindPropertyByName(FName(TEXT("SearchBounceData")));

    return *SearchBounceData->ContainerPtrToValuePtr<FFortSearchBounceData>(this);
}
