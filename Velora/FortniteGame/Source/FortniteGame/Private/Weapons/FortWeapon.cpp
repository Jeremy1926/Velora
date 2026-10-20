#include "pch.h"
#include "Weapons/FortWeapon.h"
#include "Player/FortPlayerController.h"
#include "Items/FortQuickBars.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortWeapon::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeapon"));

    return Class;
}

void AFortWeapon::PlayWeaponFireFX(bool bPersistentFire)
{
    static auto PlayWeaponFireFX = Anchor::VirtualFunction<void (*)(AFortWeapon*, bool)>(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .Event(TEXT("OnPlayWeaponFireFX"))
        .Log(TEXT("AFortWeapon::OnPlayWeaponFireFX"))
        .Caller()
        .Log(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortWeapon"))
        .Get();

    PlayWeaponFireFX(this, bPersistentFire);
}

bool AFortWeapon::CheckPendingDestroyedDueToDurability()
{
    DETOUR(AFortWeapon, CheckPendingDestroyedDueToDurability, Anchor::Hook()
        .FieldPattern("F6 ? %b %m", (UBoolProperty*)AFortWeapon::StaticClass()->FindPropertyByName(FName(TEXT("bPendingDestroyDueToDurabilityOrStackCount"))))
        .Start()
        .Inline());

    auto Instigator = GetInstigator();
    auto PlayerController = Instigator ? Cast<AFortPlayerController>(Instigator->GetController()) : nullptr;
    auto QuickBars = PlayerController ? PlayerController->GetQuickBars() : nullptr;

    if (QuickBars && GetPendingDestroyDueToDurabilityOrStackCount())
    {
        for (auto QuickBarType : { EFortQuickBars::Primary, EFortQuickBars::Secondary })
        {
            auto& QuickBar = QuickBars->GetQuickBar(QuickBarType);
            auto FocusedSlot = QuickBar.CurrentFocusedSlot;

            if (FocusedSlot >= 0 && FocusedSlot < QuickBar.Slots.Num() && !QuickBar.Slots[FocusedSlot].Items.Num() && QuickBars->TrySetFocused(QuickBarType, QuickBar.PreviousFocusedSlot, false) == -1)
                QuickBars->TrySetFocused(QuickBarType, -2, false);
        }
    }

    return Original();
}

bool AFortWeapon::TryReloadAmmo()
{
    static auto TryReloadAmmo = Anchor::VirtualFunction<bool (*)(AFortWeapon*)>(TEXT("AFortWeapon::TryReloadAmmo"))
        .String(TEXT("Maximum reload count at respawn reached on %s"))
        .Start()
        .Find("call [rax+0x*]")
        .Get();

    return TryReloadAmmo(this);
}

bool AFortWeapon::TryReloadAmmoFromAbility()
{
    return TryReloadAmmo();
}

void AFortWeapon::execTryReloadAmmoFromAbility(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortWeapon, execTryReloadAmmoFromAbility, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortWeapon:TryReloadAmmoFromAbility"))
        .Is("xor al, al; ret")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortWeapon:TryReloadAmmoFromAbility")));

    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = TryReloadAmmoFromAbility();
}

bool AFortWeapon::GetPendingDestroyDueToDurabilityOrStackCount() const
{
    static auto bPendingDestroyDueToDurabilityOrStackCount = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bPendingDestroyDueToDurabilityOrStackCount")));

    return bPendingDestroyDueToDurabilityOrStackCount->GetPropertyValue_InContainer(this);
}

FGuid& AFortWeapon::GetItemEntryGuid()
{
    static auto ItemEntryGuid = GetClass()->FindPropertyByName(FName(TEXT("ItemEntryGuid")));

    return *ItemEntryGuid->ContainerPtrToValuePtr<FGuid>(this);
}

float& AFortWeapon::GetLastFireTimeVerified()
{
    static auto LastFireTimeVerified = StaticClass()->FindPropertyByName(FName(TEXT("LastFireTimeVerified")));

    return *LastFireTimeVerified->ContainerPtrToValuePtr<float>(this);
}
