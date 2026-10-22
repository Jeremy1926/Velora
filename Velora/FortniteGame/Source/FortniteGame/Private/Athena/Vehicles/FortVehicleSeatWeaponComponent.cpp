#include "pch.h"
#include "Athena/Vehicles/FortVehicleSeatWeaponComponent.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Pawns/FortPawn.h"
#include "Pawns/FortPlayerPawn.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Weapons/FortWeapon.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FWeaponSeatDefinition::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.WeaponSeatDefinition"));

    return Struct;
}

int32 FWeaponSeatDefinition::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

int32 FWeaponSeatDefinition::GetSeatIndex()
{
    static auto SeatIndex = StaticStruct()->FindPropertyByName(FName(TEXT("SeatIndex")));

    return *SeatIndex->ContainerPtrToValuePtr<int32>(this);
}

UFortWeaponItemDefinition* FWeaponSeatDefinition::GetVehicleWeapon()
{
    static auto VehicleWeapon = StaticStruct()->FindPropertyByName(FName(TEXT("VehicleWeapon")));

    return *VehicleWeapon->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this);
}

UFortWeaponItemDefinition* FWeaponSeatDefinition::GetVehicleWeaponOverride()
{
    static auto VehicleWeaponOverride = StaticStruct()->FindPropertyByName(FName(TEXT("VehicleWeaponOverride")));

    return *VehicleWeaponOverride->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this);
}

UFortWeaponItemDefinition*& FWeaponSeatDefinition::GetLastEquippedVehicleWeapon()
{
    static auto LastEquippedVehicleWeapon = StaticStruct()->FindPropertyByName(FName(TEXT("LastEquippedVehicleWeapon")));

    return *LastEquippedVehicleWeapon->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this);
}

TWeakObjectPtr<UFortWorldItem>& FWeaponSeatDefinition::GetVehicleGrantedWeaponItem()
{
    return *(TWeakObjectPtr<UFortWorldItem>*)(&GetLastEquippedVehicleWeapon() + 1);
}

TWeakObjectPtr<AFortWeapon>& FWeaponSeatDefinition::GetVehicleGrantedWeapon()
{
    return *(TWeakObjectPtr<AFortWeapon>*)(&GetVehicleGrantedWeaponItem() + 1);
}

UClass* UFortVehicleSeatWeaponComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleSeatWeaponComponent"));

    return Class;
}

void UFortVehicleSeatWeaponComponent::OnPawnEnterSeat(IFortVehicleInterface* Vehicle, int32 SeatIdx, bool bSwitchingSeats)
{
    DETOUR(UFortVehicleSeatWeaponComponent, OnPawnEnterSeat, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleSeatWeaponComponent:EquipVehicleWeapon")))
        .String(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon failed to cleanup WeaponSeatDefinition.VehicleGrantedWeaponItem properly. PawnValid:%d"))
        .Start()
        .Log(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent"), "mov r9b, 0x1; mov r8, *; mov rdx, *; mov rcx, *; ")
        .LogSlot(TEXT("UFortVehicleSeatWeaponComponent::OnPawnExitSeat"))
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent"), -(int64_t)sizeof(void*))
        .LogSlot(TEXT("UFortVehicleSeatWeaponComponent::OnPawnEnterSeat"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent")));
    DETOUR(UFortVehicleAimingWeaponComp, OnPawnEnterSeat, Anchor::Hook(&UFortVehicleSeatWeaponComponent::OnPawnEnterSeat)
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleSeatWeaponComponent:EquipVehicleWeapon")) && StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortVehicleAimingWeaponComp")))
        .String(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon failed to cleanup WeaponSeatDefinition.VehicleGrantedWeaponItem properly. PawnValid:%d"))
        .Start()
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent"), "mov r9b, 0x1; mov r8, *; mov rdx, *; mov rcx, *; ")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent"), -(int64_t)sizeof(void*))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortVehicleAimingWeaponComp")));

    static auto AimingWeaponComp = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleAimingWeaponComp"));

    if (AimingWeaponComp && IsA(AimingWeaponComp))
        ORIGINAL_OF(UFortVehicleAimingWeaponComp, OnPawnEnterSeat)(Vehicle, SeatIdx, bSwitchingSeats);
    else
        Original(Vehicle, SeatIdx, bSwitchingSeats);

    for (auto& WeaponSeatDefinition : GetWeaponSeatDefinitions())
    {
        if (WeaponSeatDefinition.GetSeatIndex() != SeatIdx)
            continue;

        if (auto PlayerPawn = Vehicle->GetPawnAtSeat(SeatIdx))
            EquipVehicleWeapon(PlayerPawn, WeaponSeatDefinition, 0);
    }
}

void UFortVehicleSeatWeaponComponent::OnPawnExitSeat(IFortVehicleInterface* Vehicle, AFortPlayerPawn* PlayerPawn, int32 SeatIdx)
{
    static auto OnPawnExitSeat = Anchor::VirtualFunction<void (*)(UFortVehicleSeatWeaponComponent*, IFortVehicleInterface*, AFortPlayerPawn*, int32)>(TEXT("UFortVehicleSeatWeaponComponent::OnPawnExitSeat"))
        .String(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon failed to cleanup WeaponSeatDefinition.VehicleGrantedWeaponItem properly. PawnValid:%d"))
        .Start()
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent"), "mov r9b, 0x1; mov r8, *; mov rdx, *; mov rcx, *; ")
        .Get();

    OnPawnExitSeat(this, Vehicle, PlayerPawn, SeatIdx);
}

void UFortVehicleSeatWeaponComponent::EquipVehicleWeapon(AFortPawn* FortPawn, FWeaponSeatDefinition& WeaponSeatDefinition, int32 ItemLevel)
{
    auto PlayerController = FortPawn ? Cast<AFortPlayerControllerAthena>(FortPawn->GetController()) : nullptr;
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;
    auto VehicleWeapon = WeaponSeatDefinition.GetVehicleWeaponOverride() ? WeaponSeatDefinition.GetVehicleWeaponOverride() : WeaponSeatDefinition.GetVehicleWeapon();

    if (!VehicleWeapon || !WorldInventory || !FortPawn->HasAuthority())
        return;

    auto CachedWeaponState = GetCachedWeaponState();
    auto bRestoreWeaponData = GetSaveAndRestoreWeaponData() && CachedWeaponState && CachedWeaponState->bHasPrevious;
    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(VehicleWeapon, 1, ItemLevel);
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    if (bRestoreWeaponData)
        ItemEntry->SetLoadedAmmo(CachedWeaponState->AmmoInClip);
    else if (auto WeaponStats = VehicleWeapon->GetWeaponStats<FFortBaseWeaponStats>())
        ItemEntry->SetLoadedAmmo(VehicleWeapon->GetBaseClipSizeWithMods(WeaponStats, ItemEntry->GetLevel()));

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;
    WorldInventory->SetItemRequiresUpdate(*ItemEntry);
    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    FGuid TrackerGuid;

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
        {
            WeaponSeatDefinition.GetVehicleGrantedWeaponItem() = Item;
            TrackerGuid = Item->GetTrackerGuid();
        }
    }

    WeaponSeatDefinition.GetLastEquippedVehicleWeapon() = VehicleWeapon;

    auto Weapon = FortPawn->EquipWeaponDefinition(VehicleWeapon, ItemGuid, TrackerGuid);

    WeaponSeatDefinition.GetVehicleGrantedWeapon() = Weapon;

    if (Weapon && bRestoreWeaponData)
        Weapon->GetLastFireTimeVerified() = CachedWeaponState->LastFireTime;
}

TArray<FWeaponSeatDefinition>& UFortVehicleSeatWeaponComponent::GetWeaponSeatDefinitions()
{
    static auto WeaponSeatDefinitions = StaticClass()->FindPropertyByName(FName(TEXT("WeaponSeatDefinitions")));

    return *WeaponSeatDefinitions->ContainerPtrToValuePtr<TArray<FWeaponSeatDefinition>>(this);
}

bool UFortVehicleSeatWeaponComponent::GetSaveAndRestoreWeaponData() const
{
    static auto bSaveAndRestoreWeaponData = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bSaveAndRestoreWeaponData")));

    return bSaveAndRestoreWeaponData && bSaveAndRestoreWeaponData->GetPropertyValue_InContainer(this);
}

FVehicleWeapon_RetainedData* UFortVehicleSeatWeaponComponent::GetCachedWeaponState()
{
    static auto CachedWeaponState = StaticClass()->FindPropertyByName(FName(TEXT("CachedWeaponState")));

    return CachedWeaponState ? CachedWeaponState->ContainerPtrToValuePtr<FVehicleWeapon_RetainedData>(this) : nullptr;
}
