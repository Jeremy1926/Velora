#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Building/BuildingContainer.h"
#include "Building/BuildingSMActor.h"
#include "UObject/UObjectArray.h"
#include "Items/FortPickup.h"
#include "Player/FortPlayerState.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItemDefinition.h"
#include "FortniteGame.h"
#include "FortGameData.h"
#include "Abilities/FortAbilitySet.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/FortGameplayAbility.h"
#include "GameplayAbilitySpec.h"
#include "FortKismetLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/Casts.h"
#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto SpawnDefaultPawnFor = Detour(TEXT("AFortGameModeAthena::SpawnDefaultPawnFor_Implementation"), Scanner::String(TEXT("SpawnDefaultPawnFor: Failed due to no world")).Start().Get(), &AFortGameModeAthena::SpawnDefaultPawnFor_Implementation);

static auto OnGivenMatchAssignment = Detour(TEXT("AFortGameModeAthena::OnGivenMatchAssignment"), Scanner::String(TEXT("match.option.warmupRequiredPlayerCount")).Start().Get(), &AFortGameModeAthena::OnGivenMatchAssignment);

UClass* AFortGameModeAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));

    return Class;
}

APawn* AFortGameModeAthena::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    auto PlayerController = Cast<AFortPlayerControllerAthena>(NewPlayer);

    if (!PlayerController)
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no player controller"));
        return nullptr;
    }

    if (!StartSpot)
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no start spot"));
        return nullptr;
    }

    if (!GetDefaultPawnClassForController(NewPlayer))
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no default pawn class"));
        return nullptr;
    }

    if (!GetWorld())
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no world"));
        return nullptr;
    }

    auto Pawn = AGameModeBase::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
    auto PlayerState = Cast<AFortPlayerState>(NewPlayer->GetPlayerState());

    if (Pawn && PlayerState)
    {
        PlayerState->ApplyCharacterCustomization(Pawn);

        if (auto AbilitySet = UFortGameData::Get()->GetGenericPlayerAbilitySet())
        {
            for (auto& GameplayAbility : AbilitySet->GetGameplayAbilities())
                PlayerState->GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(GameplayAbility.GetDefaultObject(), 1, INDEX_NONE, AbilitySet));
        }

        auto CustomizationLoadout = PlayerController->GetCustomizationLoadout();

        if (auto Pickaxe = CustomizationLoadout ? CustomizationLoadout->GetPickaxe() : nullptr)
        {
            if (!PlayerController->K2_FindExistingItemForDefinition(Pickaxe->GetWeaponDefinition(), false))
                UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, Pickaxe->GetWeaponDefinition(), 1);
        }

        for (auto& StartingItem : GetStartingItems())
        {
            if (!PlayerController->K2_FindExistingItemForDefinition(StartingItem.Item, false))
                UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, Cast<UFortWorldItemDefinition>(StartingItem.Item), StartingItem.Count);
        }
    }

    return Pawn;
}

void AFortGameModeAthena::OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions)
{
    static auto SpawnLoot = []
    {
        auto BounceContainer = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer")).Find("jmp [*]").Disp();
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingContainer"));

        return DetourVTable(TEXT("ABuildingContainer::SpawnLoot"), (void**)(VTable + BounceContainer + sizeof(void*)), &ABuildingContainer::SpawnLoot);
    }();

    static auto GivePickupTo = []
    {
        auto GivePickupTo = Scanner::String(TEXT("%s did not have PickupTargetInventoryOwnerInterface to pickup %s, already dead: %d")).Start().Find("jmp [*+0x*]").Disp();
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickup"));
        auto AthenaVTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickupAthena"));

        DetourVTable(TEXT("AFortPickupAthena::GivePickupTo"), (void**)(AthenaVTable + GivePickupTo), &AFortPickup::GivePickupTo);

        return DetourVTable(TEXT("AFortPickup::GivePickupTo"), (void**)(VTable + GivePickupTo), &AFortPickup::GivePickupTo);
    }();

    static auto AttemptSpawnResources = []
    {
        auto AttemptSpawnResources = Scanner::String("OwnedBuildingDamaged").Find("call [*+0x*]", 2).Disp();
        auto BuildingSMActorClass = ABuildingSMActor::StaticClass();
        auto Stub = (*(void***)BuildingSMActorClass->GetDefaultObject(false))[AttemptSpawnResources / sizeof(void*)];

        auto Classes = 0;

        for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(BuildingSMActorClass))
                continue;

            if (auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false))
            {
                auto Slot = *(void***)DefaultObject + AttemptSpawnResources / sizeof(void*);

                if (*Slot == Stub)
                {
                    DetourVTable(nullptr, Slot, &ABuildingSMActor::AttemptSpawnResources);
                    Classes++;
                }
            }
        }

        UE_LOG(LogVelora, Log, TEXT("Hooked ABuildingSMActor::AttemptSpawnResources in %d classes"), Classes);

        return AttemptSpawnResources;
    }();

    static auto BeginPlay = []
    {
        auto ReceiveBeginPlayName = Scanner::String(TEXT("ReceiveBeginPlay")).Find("lea *, [0x*]").Target();
        auto ReceiveBeginPlay = Scanner::Reference(ReceiveBeginPlayName, "mov rdx, *").Start().Get();
        auto ActorBeginPlay = Scanner::CallTo(ReceiveBeginPlay).Start().Get();
        auto ActorVTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = 0;

        while (ActorVTable[Index] != ActorBeginPlay)
            Index++;

        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingContainer"));

        return DetourVTable(TEXT("ABuildingContainer::BeginPlay"), (void**)(VTable + Index * sizeof(void*)), &ABuildingContainer::BeginPlay);
    }();

    (this->*::OnGivenMatchAssignment)(MatchOptions);

    TArray<AActor*> BuildingContainers;
    UGameplayStatics::GetAllActorsOfClass(this, ABuildingContainer::StaticClass(), BuildingContainers);

    for (auto Actor : BuildingContainers)
    {
        auto BuildingContainer = (ABuildingContainer*)Actor;

        if (BuildingContainer->GetStartAlreadySearched_Athena())
            BuildingContainer->SpawnLoot(nullptr);
    }
}

FName AFortGameModeAthena::RedirectLootTierGroup(FName LootTierGroup) const
{
    static auto RedirectLootTierGroup = (FName* (*)(const AFortGameModeAthena*, FName*, FName))Scanner::String("Loot_AthenaSupplyDrop").Start().Get();

    FName Result(NAME_None);

    return *RedirectLootTierGroup(this, &Result, LootTierGroup);
}
