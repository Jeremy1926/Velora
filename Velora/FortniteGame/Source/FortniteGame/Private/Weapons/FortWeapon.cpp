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

static auto CheckPendingDestroyedDueToDurability = []
{
    auto bPendingDestroyDueToDurabilityOrStackCount = (UBoolProperty*)AFortWeapon::StaticClass()->FindPropertyByName(FName(TEXT("bPendingDestroyDueToDurabilityOrStackCount")));

    if (!bPendingDestroyDueToDurabilityOrStackCount)
        return Detour(TEXT("AFortWeapon::CheckPendingDestroyedDueToDurability"), nullptr, &AFortWeapon::CheckPendingDestroyedDueToDurability);

    auto Offset = bPendingDestroyDueToDurabilityOrStackCount->GetOffset_ForInternal() + bPendingDestroyDueToDurabilityOrStackCount->GetByteOffset();
    auto Signature = std::format("F6 ? {:02X} {:02X} {:02X} {:02X} {:02X}", Offset & 0xFF, (Offset >> 8) & 0xFF, (Offset >> 16) & 0xFF, (Offset >> 24) & 0xFF, bPendingDestroyDueToDurabilityOrStackCount->GetFieldMask());

    return Detour(TEXT("AFortWeapon::CheckPendingDestroyedDueToDurability"), Scanner::Pattern(Signature.c_str()).Start().Get(), &AFortWeapon::CheckPendingDestroyedDueToDurability);
}();

UClass* AFortWeapon::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeapon"));

    return Class;
}

void AFortWeapon::PlayWeaponFireFX(bool bPersistentFire)
{
    static auto PlayWeaponFireFX = []
    {
        auto OnPlayWeaponFireFX = Scanner::Reference(Scanner::String(TEXT("OnPlayWeaponFireFX")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get(TEXT("AFortWeapon::OnPlayWeaponFireFX"));
        auto Function = Scanner::CallTo(OnPlayWeaponFireFX).Start().Get(TEXT("AFortWeapon::PlayWeaponFireFX"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWeapon"));

        return Scanner::Slot(TEXT("AFortWeapon::PlayWeaponFireFX"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == Function; });
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortWeapon*, bool))(VTable + PlayWeaponFireFX))(this, bPersistentFire);
}

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

    return (this->*::CheckPendingDestroyedDueToDurability)();
}

bool AFortWeapon::GetPendingDestroyDueToDurabilityOrStackCount() const
{
    static auto bPendingDestroyDueToDurabilityOrStackCount = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bPendingDestroyDueToDurabilityOrStackCount")));

    return bPendingDestroyDueToDurabilityOrStackCount->GetPropertyValue_InContainer(this);
}
