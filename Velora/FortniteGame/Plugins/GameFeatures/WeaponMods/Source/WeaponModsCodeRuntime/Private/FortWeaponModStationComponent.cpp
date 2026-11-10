#include "pch.h"
#include "FortWeaponModStationComponent.h"
#include "Weapons/FortWeaponModItemDefinition.h"
#include "Weapons/FortWeapon.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Items/FortWorldItemDefinition.h"
#include "Player/FortPlayerController.h"
#include "FortKismetLibrary.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Controller.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

bool AFortWeaponModStationBase::GetModCostOverride(float& OutCost)
{
    static auto GetModCostOverride = Anchor::Optional<bool (*)(AFortWeaponModStationBase*, float&)>(TEXT("AFortWeaponModStationBase::GetModCostOverride"))
        .Event(TEXT("GetModCostOverride"))
        .Get();

    return GetModCostOverride && GetModCostOverride(this, OutCost);
}

UClass* UFortWeaponModStationComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent"));

    return Class;
}

void UFortWeaponModStationComponent::ServerPurchaseWeaponModForWeapon_Implementation(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon)
{
    DETOUR(UFortWeaponModStationComponent, ServerPurchaseWeaponModForWeapon_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent:ServerPurchaseWeaponModForWeapon"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Optional()
        .InVTable(TEXT("/Script/WeaponModsCodeRuntime.Default__FortWeaponModStationComponent")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());

    if (!PlayerController || !Weapon || !WeaponMod)
        return;

    if (!Weapon->CanAddWeaponMod(WeaponMod))
        return;

    if (!CanPlayerAffordModForWeapon(WeaponMod, Weapon, PlayerController))
        return;

    auto TransactionCurrency = GetTransactionCurrency();
    auto WeaponModStation = GetCurrentInteractingWeaponModStation();
    float OverrideCost = 0.0f;
    const bool bCostOverridden = WeaponModStation && WeaponModStation->GetModCostOverride(OverrideCost);
    const float Cost = bCostOverridden ? OverrideCost : GetCostForWeaponMod(WeaponMod);

    if (!bCostOverridden || OverrideCost != 0.0f)
    {
        if (!(Cost > 0.0f) || !TransactionCurrency)
            return;

        if (UFortKismetLibrary::K2_RemoveItemFromPlayer(PlayerController, (UFortWorldItemDefinition*)TransactionCurrency, (int32)Cost, false) <= 0)
            return;
    }

    if (!UFortWeaponModFunctionLibrary::TryAddWeaponMod(WeaponMod, Weapon))
        return;

    if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()))
        UFortWeaponModFunctionLibrary::AssignPlayerNameToWeapon(PlayerState, Weapon);

    UFortWeaponModFunctionLibrary::RemoveModSetNameFromWeapon(Weapon);
}

void UFortWeaponModStationComponent::ServerPurchaseRemoveMod_Implementation(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon)
{
    DETOUR(UFortWeaponModStationComponent, ServerPurchaseRemoveMod_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent:ServerPurchaseRemoveMod"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Optional()
        .InVTable(TEXT("/Script/WeaponModsCodeRuntime.Default__FortWeaponModStationComponent")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());

    if (!PlayerController || !CanPlayerAffordRemoveModFromWeapon(WeaponMod, Weapon, PlayerController))
        return;

    if (!Weapon || !WeaponMod)
        return;

    const int32 Cost = (int32)GetRemoveModCost().GetValueAtLevel(0.0f);

    if (Cost > 0)
    {
        if (UFortKismetLibrary::K2_RemoveItemFromPlayer(PlayerController, (UFortWorldItemDefinition*)GetTransactionCurrency(), Cost, false) != Cost)
            return;
    }

    if (!UFortWeaponModFunctionLibrary::TryRemoveWeaponMod(WeaponMod, Weapon))
        return;

    auto DefaultIronSightMod = GetDefaultIronSightMod();
    auto DefaultMagazineMod = GetDefaultMagazineMod();

    if (DefaultIronSightMod && WeaponMod->GetModSlot() == DefaultIronSightMod->GetModSlot())
    {
        UFortWeaponModFunctionLibrary::TryAddWeaponMod(DefaultIronSightMod, Weapon);
    }
    else if (DefaultMagazineMod && WeaponMod->GetModSlot() == DefaultMagazineMod->GetModSlot())
    {
        UFortWeaponModFunctionLibrary::TryAddWeaponMod(DefaultMagazineMod, Weapon);
    }

    if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()))
        UFortWeaponModFunctionLibrary::AssignPlayerNameToWeapon(PlayerState, Weapon);

    UFortWeaponModFunctionLibrary::RemoveModSetNameFromWeapon(Weapon);
}

bool UFortWeaponModStationComponent::CanPlayerAffordModForWeapon(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon, AController* Controller)
{
    static auto CanPlayerAffordModForWeapon = Anchor::Function<bool (*)(UFortWeaponModStationComponent*, UFortWeaponModItemDefinition*, AFortWeapon*, AController*)>(TEXT("UFortWeaponModStationComponent::CanPlayerAffordModForWeapon"))
        .Event(TEXT("CanPlayerAffordModForWeapon"))
        .Get();

    return CanPlayerAffordModForWeapon(this, WeaponMod, Weapon, Controller);
}

bool UFortWeaponModStationComponent::CanPlayerAffordRemoveModFromWeapon(UFortWeaponModItemDefinition* WeaponMod, AFortWeapon* Weapon, AController* Controller)
{
    static auto CanPlayerAffordRemoveModFromWeapon = Anchor::Function<bool (*)(UFortWeaponModStationComponent*, UFortWeaponModItemDefinition*, AFortWeapon*, AController*)>(TEXT("UFortWeaponModStationComponent::CanPlayerAffordRemoveModFromWeapon"))
        .Event(TEXT("CanPlayerAffordRemoveModFromWeapon"))
        .Get();

    return CanPlayerAffordRemoveModFromWeapon(this, WeaponMod, Weapon, Controller);
}

float UFortWeaponModStationComponent::GetCostForWeaponMod(UFortWeaponModItemDefinition* WeaponMod)
{
    static auto GetCostForWeaponMod = Anchor::Function<float (*)(UFortWeaponModStationComponent*, UFortWeaponModItemDefinition*)>(TEXT("UFortWeaponModStationComponent::GetCostForWeaponMod"))
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent:CanPlayerAffordModForWeapon"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/WeaponModsCodeRuntime.Default__FortWeaponModStationComponent"))
        .Find("call 0x*")
        .Target()
        .Get();

    return GetCostForWeaponMod(this, WeaponMod);
}

UFortItemDefinition* UFortWeaponModStationComponent::GetTransactionCurrency()
{
    static auto GetTransactionCurrency = Anchor::Optional<UFortItemDefinition* (*)(UFortWeaponModStationComponent*)>(TEXT("UFortWeaponModStationComponent::GetTransactionCurrency"))
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent:CanPlayerAffordModForWeapon"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/WeaponModsCodeRuntime.Default__FortWeaponModStationComponent"))
        .Find("call 0x*; mov rcx, *; movaps xmm6, xmm0; call 0x*")
        .Call(2)
        .Get();

    if (GetTransactionCurrency)
        return GetTransactionCurrency(this);

    static auto GetCurrencyOwner = Anchor::Function<UObject* (*)()>(TEXT("UFortWeaponModStationComponent::CanPlayerAffordModForWeapon_Implementation currency owner"))
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent:CanPlayerAffordModForWeapon"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/WeaponModsCodeRuntime.Default__FortWeaponModStationComponent"))
        .Find("call 0x*; movaps xmm6, xmm0; call 0x*; mov rcx, rax; call 0x*")
        .Call(2)
        .Get();

    static auto GetCurrency = Anchor::Function<UFortItemDefinition* (*)(UObject*)>(TEXT("UFortWeaponModStationComponent::CanPlayerAffordModForWeapon_Implementation currency"))
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent:CanPlayerAffordModForWeapon"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/WeaponModsCodeRuntime.Default__FortWeaponModStationComponent"))
        .Find("call 0x*; movaps xmm6, xmm0; call 0x*; mov rcx, rax; call 0x*")
        .Call(3)
        .Get();

    return GetCurrency(GetCurrencyOwner());
}

AFortWeaponModStationBase* UFortWeaponModStationComponent::GetCurrentInteractingWeaponModStation() const
{
    static auto CurrentInteractingWeaponModStation = StaticClass()->FindPropertyByName(FName(TEXT("CurrentInteractingWeaponModStation")));

    return CurrentInteractingWeaponModStation ? *CurrentInteractingWeaponModStation->ContainerPtrToValuePtr<AFortWeaponModStationBase*>(this) : nullptr;
}

const FScalableFloat& UFortWeaponModStationComponent::GetRemoveModCost() const
{
    static auto RemoveModCost = StaticClass()->FindPropertyByName(FName(TEXT("RemoveModCost")));

    return *RemoveModCost->ContainerPtrToValuePtr<FScalableFloat>(this);
}

UFortWeaponModItemDefinition* UFortWeaponModStationComponent::GetDefaultIronSightMod() const
{
    static auto DefaultIronSightMod = StaticClass()->FindPropertyByName(FName(TEXT("DefaultIronSightMod")));

    return DefaultIronSightMod ? *DefaultIronSightMod->ContainerPtrToValuePtr<UFortWeaponModItemDefinition*>(this) : nullptr;
}

UFortWeaponModItemDefinition* UFortWeaponModStationComponent::GetDefaultMagazineMod() const
{
    static auto DefaultMagazineMod = StaticClass()->FindPropertyByName(FName(TEXT("DefaultMagazineMod")));

    return DefaultMagazineMod ? *DefaultMagazineMod->ContainerPtrToValuePtr<UFortWeaponModItemDefinition*>(this) : nullptr;
}

UClass* UFortPickupInteractOverrideComponent_WeaponMods::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/WeaponModsCodeRuntime.FortPickupInteractOverrideComponent_WeaponMods"));

    return Class;
}

void UFortPickupInteractOverrideComponent_WeaponMods::ServerApplyWeaponModFromPickup_Implementation(AFortPickup* Pickup, UFortWeaponModItemDefinition* WeaponModDef, AFortWeapon* Weapon)
{
    DETOUR(UFortPickupInteractOverrideComponent_WeaponMods, ServerApplyWeaponModFromPickup_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortPickupInteractOverrideComponent_WeaponMods:ServerApplyWeaponModFromPickup"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Optional()
        .InVTable(TEXT("/Script/WeaponModsCodeRuntime.Default__FortPickupInteractOverrideComponent_WeaponMods")));

    Original(Pickup, WeaponModDef, Weapon);

    if (Weapon)
        Weapon->UpdateCurrentWeaponModSlotsOnWorldItem();
}
