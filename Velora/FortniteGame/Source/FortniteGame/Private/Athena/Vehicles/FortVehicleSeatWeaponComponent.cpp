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

static int32 OnPawnEnterSeatSlot()
{
    static auto OnPawnEnterSeat = []
    {
        auto UnEquipVehicleWeapon = Scanner::String(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon failed to cleanup WeaponSeatDefinition.VehicleGrantedWeaponItem properly. PawnValid:%d")).Start().Get(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon"));
        auto UnEquipLastWeapon = std::format("mov r9b, 0x1; mov r8, *; mov rdx, *; mov rcx, *; call {:#x}", (uintptr_t)UnEquipVehicleWeapon);
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent"));
        auto OnPawnExitSeat = Scanner::Slot(TEXT("UFortVehicleSeatWeaponComponent::OnPawnExitSeat"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Has(UnEquipLastWeapon); });

        return Scanner::Slot(TEXT("UFortVehicleSeatWeaponComponent::OnPawnEnterSeat"), Scanner::IsSlot(OnPawnExitSeat) ? OnPawnExitSeat - (int32)sizeof(void*) : 0);
    }();

    return OnPawnEnterSeat;
}

static auto OnPawnEnterSeat = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleSeatWeaponComponent:EquipVehicleWeapon")) ? DetourVTable(TEXT("UFortVehicleSeatWeaponComponent::OnPawnEnterSeat"), *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortVehicleSeatWeaponComponent")), OnPawnEnterSeatSlot(), &UFortVehicleSeatWeaponComponent::OnPawnEnterSeat) : nullptr;

static auto AimingWeaponCompOnPawnEnterSeat = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleSeatWeaponComponent:EquipVehicleWeapon")) && StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortVehicleAimingWeaponComp")) ? DetourVTable(TEXT("UFortVehicleAimingWeaponComp::OnPawnEnterSeat"), *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortVehicleAimingWeaponComp")), OnPawnEnterSeatSlot(), &UFortVehicleSeatWeaponComponent::OnPawnEnterSeat) : nullptr;

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
    static auto AimingWeaponComp = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleAimingWeaponComp"));

    (this->*(AimingWeaponComp && IsA(AimingWeaponComp) ? ::AimingWeaponCompOnPawnEnterSeat : ::OnPawnEnterSeat))(Vehicle, SeatIdx, bSwitchingSeats);

    for (auto& WeaponSeatDefinition : GetWeaponSeatDefinitions())
    {
        if (WeaponSeatDefinition.GetSeatIndex() != SeatIdx)
            continue;

        if (auto PlayerPawn = Vehicle->GetPawnAtSeat(SeatIdx))
            EquipVehicleWeapon(PlayerPawn, WeaponSeatDefinition, 0);
    }
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

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
            WeaponSeatDefinition.GetVehicleGrantedWeaponItem() = Item;
    }

    WeaponSeatDefinition.GetLastEquippedVehicleWeapon() = VehicleWeapon;

    auto Weapon = FortPawn->EquipWeaponDefinition(VehicleWeapon, ItemGuid);

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
