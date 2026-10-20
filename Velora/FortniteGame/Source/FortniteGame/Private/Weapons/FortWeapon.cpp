#include "pch.h"
#include "Weapons/FortWeapon.h"
#include "Player/FortPlayerController.h"
#include "Items/FortQuickBars.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
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
    static auto PlayWeaponFireFX = Anchor::Slot(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .Event(TEXT("OnPlayWeaponFireFX"))
        .Log(TEXT("AFortWeapon::OnPlayWeaponFireFX"))
        .Caller()
        .Log(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortWeapon"))
        .Get();

    Scanner::Virtual<void (*)(AFortWeapon*, bool)>(this, PlayWeaponFireFX)(this, bPersistentFire);
}

DETOUR(AFortWeapon, CheckPendingDestroyedDueToDurability, Anchor::Hook()
    .FieldPattern("F6 ? %b %m", (UBoolProperty*)AFortWeapon::StaticClass()->FindPropertyByName(FName(TEXT("bPendingDestroyDueToDurabilityOrStackCount"))))
    .Start()
    .Inline())
bool AFortWeapon::CheckPendingDestroyedDueToDurability()
{
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
