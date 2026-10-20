#include "pch.h"
#include "Athena/Abilities/FortGameplayAbilityAthena_PeriodicItemGrant.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "Player/FortPlayerController.h"
#include "FortKismetLibrary.h"
#include "GameplayEffectTypes.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

struct FPendingItemAward
{
    TWeakObjectPtr<UFortGameplayAbilityAthena_PeriodicItemGrant> Ability;
    const FActiveItemGrantInfo* GrantInfo;
    float Rate;
    float AwardTime;
};

static TArray<FPendingItemAward> PendingItemAwards;

static auto execStartItemAwardTimers = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant:StartItemAwardTimers"));

    if (!Function)
        return false;

    auto Finish = Scanner::Exec(TEXT("/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant:StartItemAwardTimers")).Find("mov [*+0x20], *; jmp 0x*");

    if (!Finish.Get() || !Scanner::At(Finish.Find("jmp 0x*").Target()).Is("ret*"))
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked UFortGameplayAbilityAthena_PeriodicItemGrant::execStartItemAwardTimers, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&UFortGameplayAbilityAthena_PeriodicItemGrant::execStartItemAwardTimers);

    return true;
}();

static auto execStopItemAwardTimers = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant:StopItemAwardTimers"));

    if (!Function)
        return false;

    auto Finish = Scanner::Exec(TEXT("/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant:StopItemAwardTimers")).Find("mov [*+0x20], *; jmp 0x*");

    if (!Finish.Get() || !Scanner::At(Finish.Find("jmp 0x*").Target()).Is("ret*"))
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked UFortGameplayAbilityAthena_PeriodicItemGrant::execStopItemAwardTimers, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&UFortGameplayAbilityAthena_PeriodicItemGrant::execStopItemAwardTimers);

    return true;
}();

UScriptStruct* FActiveItemGrantInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ActiveItemGrantInfo"));

    return Struct;
}

UFortWorldItemDefinition* FActiveItemGrantInfo::GetItem() const
{
    static auto Item = StaticStruct()->FindPropertyByName(FName(TEXT("Item")));

    return *Item->ContainerPtrToValuePtr<UFortWorldItemDefinition*>(this);
}

const FScalableFloat& FActiveItemGrantInfo::GetAmountToGive() const
{
    static auto AmountToGive = StaticStruct()->FindPropertyByName(FName(TEXT("AmountToGive")));

    return *AmountToGive->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& FActiveItemGrantInfo::GetMaxAmount() const
{
    static auto MaxAmount = StaticStruct()->FindPropertyByName(FName(TEXT("MaxAmount")));

    return *MaxAmount->ContainerPtrToValuePtr<FScalableFloat>(this);
}

UClass* UFortGameplayAbilityAthena_PeriodicItemGrant::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbilityAthena_PeriodicItemGrant"));

    return Class;
}

void UFortGameplayAbilityAthena_PeriodicItemGrant::StartItemAwardTimers()
{
    auto Pawn = Cast<AFortPawn>(GetAvatarActorFromActorInfo());
    auto PlayerController = Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;

    if (!PlayerController || !Pawn->HasAuthority())
        return;

    GetOwnerControllerCachedValue() = PlayerController;

    static auto ItemsToGrant = GetClass()->FindPropertyByName(FName(TEXT("ItemsToGrant")));
    static auto ValueAlignment = FScalableFloat::StaticStruct()->GetMinAlignment();
    static auto PairAlignment = FMath::Max(FMath::Max(FActiveItemGrantInfo::StaticStruct()->GetMinAlignment(), ValueAlignment), (int32)sizeof(int32));
    static auto ValueOffset = (FActiveItemGrantInfo::StaticStruct()->GetPropertiesSize() + ValueAlignment - 1) & ~(ValueAlignment - 1);
    static auto PairSize = (ValueOffset + FScalableFloat::StaticStruct()->GetPropertiesSize() + PairAlignment - 1) & ~(PairAlignment - 1);
    static auto ElementSize = (PairSize + 2 * (int32)sizeof(int32) + PairAlignment - 1) & ~(PairAlignment - 1);

    auto& Elements = *ItemsToGrant->ContainerPtrToValuePtr<TSparseArray<uint8>>(this);
    auto ElementData = *ItemsToGrant->ContainerPtrToValuePtr<const uint8*>(this);

    for (int32 Index = 0; Index < Elements.GetMaxIndex(); Index++)
    {
        if (!Elements.IsAllocated(Index))
            continue;

        auto GrantInfo = (const FActiveItemGrantInfo*)(ElementData + Index * ElementSize);
        const float Rate = ((const FScalableFloat*)((const uint8*)GrantInfo + ValueOffset))->GetValueAtLevel(0.f);

        if (Rate > 0.f)
            PendingItemAwards.Add(FPendingItemAward{ this, GrantInfo, Rate, Pawn->GetWorld()->GetTimeSeconds() + Rate });
    }
}

void UFortGameplayAbilityAthena_PeriodicItemGrant::StopItemAwardTimers()
{
    for (int32 Index = PendingItemAwards.Num() - 1; Index >= 0; Index--)
    {
        if (PendingItemAwards[Index].Ability.Get() == this)
            PendingItemAwards.RemoveAt(Index);
    }
}

void UFortGameplayAbilityAthena_PeriodicItemGrant::AwardItem(const FActiveItemGrantInfo& GrantInfo)
{
    auto PlayerController = GetOwnerControllerCachedValue();
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;
    auto ItemDefinition = GrantInfo.GetItem();

    if (!WorldInventory || !ItemDefinition)
        return;

    auto& Inventory = WorldInventory->GetInventory();
    FFortItemEntry* ExistingItemEntry = nullptr;
    int32 CurrentAmount = 0;

    for (auto& ItemEntry : Inventory.GetReplicatedEntries())
    {
        if (ItemEntry.GetItemDefinition() != ItemDefinition)
            continue;

        if (!ExistingItemEntry)
            ExistingItemEntry = &ItemEntry;

        CurrentAmount += ItemEntry.GetCount();
    }

    const int32 NumberToGive = FMath::Min((int32)GrantInfo.GetAmountToGive().GetValueAtLevel(0.f), (int32)GrantInfo.GetMaxAmount().GetValueAtLevel(0.f) - CurrentAmount);

    if (NumberToGive <= 0)
        return;

    if (!ExistingItemEntry)
    {
        UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, ItemDefinition, NumberToGive);
        return;
    }

    auto Pawn = Cast<AFortPawn>(GetAvatarActorFromActorInfo());
    const int32 NumberToStack = FMath::Min(NumberToGive, ItemDefinition->GetMaxStackSize(Pawn ? Pawn->GetAbilitySystemComponent() : nullptr) - ExistingItemEntry->GetCount());

    if (NumberToStack <= 0)
        return;

    ExistingItemEntry->SetCount(ExistingItemEntry->GetCount() + NumberToStack);
    Inventory.MarkItemDirty(*ExistingItemEntry);

    WorldInventory->HandleInventoryLocalUpdate();
}

void UFortGameplayAbilityAthena_PeriodicItemGrant::execStartItemAwardTimers(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    StartItemAwardTimers();
}

void UFortGameplayAbilityAthena_PeriodicItemGrant::execStopItemAwardTimers(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    StopItemAwardTimers();
}

void UFortGameplayAbilityAthena_PeriodicItemGrant::TickItemAwardTimers()
{
    for (int32 Index = PendingItemAwards.Num() - 1; Index >= 0; Index--)
    {
        auto Ability = PendingItemAwards[Index].Ability.Get();
        auto PlayerController = Ability ? Ability->GetOwnerControllerCachedValue() : nullptr;

        if (!PlayerController)
        {
            PendingItemAwards.RemoveAt(Index);
            continue;
        }

        if (PlayerController->GetWorld()->GetTimeSeconds() < PendingItemAwards[Index].AwardTime)
            continue;

        PendingItemAwards[Index].AwardTime += PendingItemAwards[Index].Rate;

        Ability->AwardItem(*PendingItemAwards[Index].GrantInfo);
    }
}

AFortPlayerController*& UFortGameplayAbilityAthena_PeriodicItemGrant::GetOwnerControllerCachedValue()
{
    static auto OwnerControllerCachedValue = GetClass()->FindPropertyByName(FName(TEXT("OwnerControllerCachedValue")));

    return *OwnerControllerCachedValue->ContainerPtrToValuePtr<AFortPlayerController*>(this);
}
