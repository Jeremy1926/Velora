#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Building/BuildingContainer.h"
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
#include "hde/hde64.h"

static auto SpawnDefaultPawnFor = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("SpawnDefaultPawnFor: Failed due to no world")))->BeginAddress, &AFortGameModeAthena::SpawnDefaultPawnFor_Implementation);

static auto OnGivenMatchAssignment = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("match.option.warmupRequiredPlayerCount")))->BeginAddress, &AFortGameModeAthena::OnGivenMatchAssignment);

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

        if (auto Pickaxe = PlayerController->GetCustomizationLoadout().GetPickaxe())
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
        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingContainer:BounceContainer")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
            {
                auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingContainer"));

                return DetourVTable((void**)(VTable + (int32_t)Instruction.disp.disp32 + sizeof(void*)), &ABuildingContainer::SpawnLoot);
            }

            Address += Length;
        }
    }();

    static auto GivePickupTo = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("%s did not have PickupTargetInventoryOwnerInterface to pickup %s, already dead: %d")));

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 4 && Instruction.modrm_mod == 2)
            {
                auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickup"));
                auto AthenaVTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPickupAthena"));

                DetourVTable((void**)(AthenaVTable + (int32_t)Instruction.disp.disp32), &AFortPickup::GivePickupTo);

                return DetourVTable((void**)(VTable + (int32_t)Instruction.disp.disp32), &AFortPickup::GivePickupTo);
            }

            Address += Length;
        }
    }();

    static auto BeginPlay = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("ReceiveBeginPlay"));
        const uint8_t* ReceiveBeginPlayName = nullptr;

        for (auto Address = Reference; !ReceiveBeginPlayName;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Address != Reference && Instruction.opcode == 0x8D && (Instruction.modrm & 0xC7) == 0x05)
                ReceiveBeginPlayName = Address + Length + (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        const uint8_t* ReceiveBeginPlay = nullptr;

        for (auto& Section : Scanner::Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Scanner::Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + 7 <= End && !ReceiveBeginPlay; Address++)
            {
                if (Address[0] == 0x48 && Address[1] == 0x8B && Address[2] == 0x15 && Address + 7 + *(int32_t*)(Address + 3) == ReceiveBeginPlayName)
                    ReceiveBeginPlay = Scanner::Base() + Scanner::FindFunction(Address)->BeginAddress;
            }
        }

        auto ActorBeginPlay = Scanner::Base() + Scanner::FindFunction(Scanner::FindCallReference(ReceiveBeginPlay))->BeginAddress;
        auto ActorVTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = 0;

        while (ActorVTable[Index] != ActorBeginPlay)
            Index++;

        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingContainer"));

        return DetourVTable((void**)(VTable + Index * sizeof(void*)), &ABuildingContainer::BeginPlay);
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
    static auto RedirectLootTierGroup = (FName* (*)(const AFortGameModeAthena*, FName*, FName))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference("Loot_AthenaSupplyDrop"))->BeginAddress);

    FName Result(NAME_None);

    return *RedirectLootTierGroup(this, &Result, LootTierGroup);
}
