#include "pch.h"
#include "Components/FortControllerComponent_RechargeWeapons.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerController.h"
#include "Weapons/FortWeapon.h"
#include "FortKismetLibrary.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Containers/SparseArray.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FCachedRechargeAmmoData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.CachedRechargeAmmoData"));

    return Struct;
}

float FCachedRechargeAmmoData::GetServerStartTime() const
{
    static auto ServerStartTime = StaticStruct()->FindPropertyByName(FName(TEXT("ServerStartTime")));

    return *ServerStartTime->ContainerPtrToValuePtr<float>(this);
}

float FCachedRechargeAmmoData::GetChargeRate() const
{
    static auto ChargeRate = StaticStruct()->FindPropertyByName(FName(TEXT("ChargeRate")));

    return *ChargeRate->ContainerPtrToValuePtr<float>(this);
}

int32 FCachedRechargeAmmoData::GetAmountToRecharge() const
{
    static auto AmountToRecharge = StaticStruct()->FindPropertyByName(FName(TEXT("AmountToRecharge")));

    return *AmountToRecharge->ContainerPtrToValuePtr<int32>(this);
}

const TSoftObjectPtr<UFortWorldItemDefinition>& FCachedRechargeAmmoData::GetAmmoItemDefinition() const
{
    static auto AmmoItemDefinition = StaticStruct()->FindPropertyByName(FName(TEXT("AmmoItemDefinition")));

    return *AmmoItemDefinition->ContainerPtrToValuePtr<TSoftObjectPtr<UFortWorldItemDefinition>>(this);
}

bool FCachedRechargeAmmoData::GetShouldRechargeAmmoToClip() const
{
    static auto bShouldRechargeAmmoToClip = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bShouldRechargeAmmoToClip")));

    return bShouldRechargeAmmoToClip->GetPropertyValue_InContainer(this);
}

UClass* UFortControllerComponent_RechargeWeapons::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_RechargeWeapons"));

    return Class;
}

void UFortControllerComponent_RechargeWeapons::TickRechargingWeapons(UWorld* World, float DeltaSeconds)
{
    auto GameState = World && StaticClass() && FCachedRechargeAmmoData::StaticStruct() ? World->GetGameState() : nullptr;

    if (!GameState)
        return;

    for (auto PlayerState : GameState->GetPlayerArray())
    {
        auto PlayerController = PlayerState ? Cast<AFortPlayerController>(PlayerState->GetOwner()) : nullptr;

        if (auto RechargeWeapons = PlayerController ? PlayerController->FindComponentByClass<UFortControllerComponent_RechargeWeapons>() : nullptr)
            RechargeWeapons->TickComponent(DeltaSeconds, LEVELTICK_All, nullptr);
    }
}

void UFortControllerComponent_RechargeWeapons::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    DETOUR(UFortControllerComponent_RechargeWeapons, TickComponent, Anchor::Hook()
        .If(StaticClass())
        .At(Scanner::Virtual<const void*>(&((UActorComponent*)UActorComponent::StaticClass()->GetDefaultObject(false))->GetPrimaryComponentTick(), sizeof(void*)))
        .Find("mulss xmm1, *")
        .Find("call [*+0x*]")
        .Or()
        .At(Scanner::Virtual<const void*>(&((UActorComponent*)UActorComponent::StaticClass()->GetDefaultObject(false))->GetPrimaryComponentTick(), sizeof(void*)))
        .Call()
        .Find("mulss xmm1, *")
        .Find("call [*+0x*]")
        .InChildVTablesCounted(StaticClass()));

    if (DETOUR_ORIGINAL(UFortControllerComponent_RechargeWeapons, TickComponent))
        Original(DeltaTime, TickType, ThisTickFunction);

    auto Owner = GetOwner();

    if (!FCachedRechargeAmmoData::StaticStruct() || !Owner || !Owner->HasAuthority() || GetPreventRecharging())
        return;

    const float ServerWorldTimeSeconds = UFortKismetLibrary::GetServerWorldTimeSeconds(this);
    TArray<FGuid> RechargedItemGuids;

    for (auto& RechargeData : GetWeaponToRechargeDataMap())
    {
        if (RechargeData.Value->GetServerStartTime() >= 0.0f && ServerWorldTimeSeconds - RechargeData.Value->GetServerStartTime() >= RechargeData.Value->GetChargeRate())
            RechargedItemGuids.Add(RechargeData.Key);
    }

    for (auto& ItemGuid : RechargedItemGuids)
    {
        HandleRechargeAmmo(ItemGuid);
        UpdateServerStartForItem(ItemGuid, true);
    }
}

void UFortControllerComponent_RechargeWeapons::OnItemInstanceAdded(UFortWorldItem* Item)
{
    static auto OnItemInstanceAdded = Anchor::VirtualFunction<void (*)(UFortControllerComponent_RechargeWeapons*, UFortWorldItem*)>(TEXT("UFortControllerComponent_RechargeWeapons::OnItemInstanceAdded"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Is("sub rsp, 0x28; test rdx, rdx; jz 0x*; call 0x*; add rsp, 0x28; ret"); })
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Is("test rdx, rdx; jnz 0x*; ret"); })
        .Get();

    if (!OnItemInstanceAdded)
        return;

    OnItemInstanceAdded(this, Item);
    AttemptSetTickEnabledForItem(Item->GetItemGuid());
}

void UFortControllerComponent_RechargeWeapons::OnItemInstanceRemoved(UFortWorldItem* Item)
{
    static auto OnItemInstanceRemoved = Anchor::VirtualFunction<void (*)(UFortControllerComponent_RechargeWeapons*, UFortWorldItem*)>(TEXT("UFortControllerComponent_RechargeWeapons::OnItemInstanceRemoved"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Is("test rdx, rdx; jz 0x*; push rbx; sub rsp, 0x30") && Entry.Has("call [rax+0x*]; lea r*, [r*+0x*]; lea r*, [r*+0x*]; call 0x*; lea r*, [r*+0x*]; lea r*, [r*+0x*]; call 0x*"); })
        .Get();

    if (OnItemInstanceRemoved)
        OnItemInstanceRemoved(this, Item);
}

void UFortControllerComponent_RechargeWeapons::OnWeaponConsumedAmmo(const FGuid& ItemGuid)
{
    DETOUR(UFortControllerComponent_RechargeWeapons, OnWeaponConsumedAmmo, Anchor::Hook()
        .If(StaticClass())
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const ::Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov r8b, 0x1"); })
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const ::Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov rcx, *; mov r8b, 0x1"); })
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const ::Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov rdx, *; mov r8b, 0x1"); })
        .InChildVTablesCounted(StaticClass()));

    Original(ItemGuid);

    AttemptSetTickEnabledForItem(ItemGuid);
}

void UFortControllerComponent_RechargeWeapons::AttemptSetTickEnabledForItem(const FGuid& ItemGuid)
{
    auto RechargeData = FindRechargeDataForItem(ItemGuid);

    if (RechargeData && RechargeData->GetServerStartTime() >= 0.0f && !GetPreventRecharging())
        SetComponentTickEnabled(true);
}

void UFortControllerComponent_RechargeWeapons::HandleRechargeAmmo(const FGuid& ItemGuid)
{
    auto RechargeData = FindRechargeDataForItem(ItemGuid);
    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!RechargeData || !WorldInventory)
        return;

    int32 MaxAmount = 0;
    int32 CurrentAmount = 0;

    GetAmmoStackInfo(ItemGuid, MaxAmount, CurrentAmount);

    const int32 AmountToRecharge = FMath::Min(RechargeData->GetAmountToRecharge(), MaxAmount - CurrentAmount);

    if (AmountToRecharge <= 0)
        return;

    if (RechargeData->GetShouldRechargeAmmoToClip())
    {
        auto Pawn = PlayerController->GetMyFortPawn();
        auto Weapon = Pawn ? Pawn->GetCurrentWeapon() : nullptr;

        if (Weapon && Weapon->GetItemEntryGuid() == ItemGuid)
        {
            Weapon->ModAmmo(AmountToRecharge);
            return;
        }

        for (auto Item : WorldInventory->GetInventory().GetItemInstances())
        {
            if (Item && Item->GetItemGuid() == ItemGuid)
                Item->SetLoadedAmmo(CurrentAmount + AmountToRecharge);
        }

        return;
    }

    auto AmmoItemDefinition = RechargeData->GetAmmoItemDefinition().LoadSynchronous();

    if (!AmmoItemDefinition)
        return;

    auto& Inventory = WorldInventory->GetInventory();

    for (auto& ItemEntry : Inventory.GetReplicatedEntries())
    {
        if (ItemEntry.GetItemDefinition() != AmmoItemDefinition)
            continue;

        ItemEntry.SetCount(ItemEntry.GetCount() + AmountToRecharge);
        Inventory.MarkItemDirty(ItemEntry);

        WorldInventory->HandleInventoryLocalUpdate();
        return;
    }

    UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, AmmoItemDefinition, AmountToRecharge);
}

void UFortControllerComponent_RechargeWeapons::GetAmmoStackInfo(const FGuid& ItemGuid, int32& OutMaxAmount, int32& OutCurrentAmount)
{
    static auto GetAmmoStackInfo = Anchor::Function<void (*)(UFortControllerComponent_RechargeWeapons*, const FGuid&, int32&, int32&)>(TEXT("UFortControllerComponent_RechargeWeapons::GetAmmoStackInfo"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov r8b, 0x1"); })
        .Find("cmp byte ptr [rax+0x*], 0x3; jnz 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov r8b, 0x1"); })
        .Find("call 0x*; mov eax, [rsp+0x*]; dec eax")
        .Target()
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov rcx, *; mov r8b, 0x1"); })
        .Find("call 0x*; mov eax, [rsp+0x*]; dec eax")
        .Target()
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov rdx, *; mov r8b, 0x1"); })
        .Find("call 0x*; mov eax, [rsp+0x*]; dec eax")
        .Target()
        .Get();

    GetAmmoStackInfo(this, ItemGuid, OutMaxAmount, OutCurrentAmount);
}

bool UFortControllerComponent_RechargeWeapons::UpdateServerStartForItem(const FGuid& ItemGuid, bool bForce)
{
    static auto UpdateServerStartForItem = Anchor::Function<bool (*)(UFortControllerComponent_RechargeWeapons*, const FGuid&, bool)>(TEXT("UFortControllerComponent_RechargeWeapons::UpdateServerStartForItem"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov r8b, 0x1"); })
        .Find("mov r8b, 0x1")
        .Find("call 0x*")
        .Target()
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov rcx, *; mov r8b, 0x1"); })
        .Find("mov r8b, 0x1")
        .Find("call 0x*")
        .Target()
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_RechargeWeapons"), [](const Anchor::FCode& Entry) { return Entry.Has("dec eax; cmp [rsp+0x*], eax; jnz 0x*; mov rdx, *; mov r8b, 0x1"); })
        .Find("mov r8b, 0x1")
        .Find("call 0x*")
        .Target()
        .Get();

    return UpdateServerStartForItem(this, ItemGuid, bForce);
}

TArray<TPair<FGuid, FCachedRechargeAmmoData*>> UFortControllerComponent_RechargeWeapons::GetWeaponToRechargeDataMap()
{
    static auto WeaponToRechargeDataMap = StaticClass()->FindPropertyByName(FName(TEXT("WeaponToRechargeDataMap")));
    static auto ValueAlignment = FCachedRechargeAmmoData::StaticStruct()->GetMinAlignment();
    static auto PairAlignment = FMath::Max(FMath::Max(ValueAlignment, (int32)alignof(FGuid)), (int32)sizeof(int32));
    static auto ValueOffset = ((int32)sizeof(FGuid) + ValueAlignment - 1) & ~(ValueAlignment - 1);
    static auto PairSize = (ValueOffset + FCachedRechargeAmmoData::StaticStruct()->GetPropertiesSize() + PairAlignment - 1) & ~(PairAlignment - 1);
    static auto ElementSize = (PairSize + 2 * (int32)sizeof(int32) + PairAlignment - 1) & ~(PairAlignment - 1);

    TArray<TPair<FGuid, FCachedRechargeAmmoData*>> Pairs;

    if (!WeaponToRechargeDataMap)
        return Pairs;

    auto& Elements = *WeaponToRechargeDataMap->ContainerPtrToValuePtr<TSparseArray<uint8>>(this);
    auto ElementData = *WeaponToRechargeDataMap->ContainerPtrToValuePtr<uint8*>(this);

    for (int32 Index = 0; Index < Elements.GetMaxIndex(); Index++)
    {
        if (Elements.IsAllocated(Index))
            Pairs.Add({ *(const FGuid*)(ElementData + Index * ElementSize), (FCachedRechargeAmmoData*)(ElementData + Index * ElementSize + ValueOffset) });
    }

    return Pairs;
}

FCachedRechargeAmmoData* UFortControllerComponent_RechargeWeapons::FindRechargeDataForItem(const FGuid& ItemGuid)
{
    for (auto& Entry : GetWeaponToRechargeDataMap())
    {
        if (Entry.Key == ItemGuid)
            return Entry.Value;
    }

    return nullptr;
}

bool UFortControllerComponent_RechargeWeapons::GetPreventRecharging() const
{
    static auto bPreventRecharging = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bPreventRecharging")));

    return bPreventRecharging && bPreventRecharging->GetPropertyValue_InContainer(this);
}
