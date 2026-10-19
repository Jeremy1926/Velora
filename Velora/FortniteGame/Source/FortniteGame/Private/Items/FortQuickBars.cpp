#include "pch.h"
#include "Items/FortQuickBars.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* AFortQuickBars::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuickBars"));

    return Class;
}

static const uint8_t* GetActivateSlot(const AFortQuickBars* QuickBars)
{
    static auto Call = Scanner::String(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d.")).Find("mov byte ptr [rsp+0x20], 0x1").Find("call *");
    static auto ActivateSlot = Call.Is("call 0x*") ? Call.Target(TEXT("AFortQuickBars::ActivateSlot")) : nullptr;
    static auto ActivateSlotSlot = ActivateSlot ? 0 : Call.Slot(TEXT("AFortQuickBars::ActivateSlot"));

    return ActivateSlot ? ActivateSlot : (*(const uint8_t***)QuickBars)[ActivateSlotSlot / sizeof(void*)];
}

void AFortQuickBars::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    Scanner::Function<void (*)(AFortQuickBars*, EFortQuickBars, int32, float, bool, bool, bool)>(GetActivateSlot(this))(this, InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot, false, false);
}

FQuickBar& AFortQuickBars::GetQuickBar(EFortQuickBars InQuickBar)
{
    static auto PrimaryQuickBar = GetClass()->FindPropertyByName(FName(TEXT("PrimaryQuickBar")));
    static auto SecondaryQuickBar = GetClass()->FindPropertyByName(FName(TEXT("SecondaryQuickBar")));

    return *(InQuickBar == EFortQuickBars::Secondary ? SecondaryQuickBar : PrimaryQuickBar)->ContainerPtrToValuePtr<FQuickBar>(this);
}

void AFortQuickBars::OnQuickBarChanged(EFortQuickBars InQuickBar)
{
    static auto OnQuickBarChanged = Scanner::Function<void (*)(AFortQuickBars*, EFortQuickBars)>(Scanner::At(Scanner::Exec(TEXT("/Script/FortniteGame.FortQuickBars:OnRep_SecondaryQuickBar")).Find("jmp 0x*").Target()).Find("jmp 0x*").Target(TEXT("AFortQuickBars::OnQuickBarChanged")));

    OnQuickBarChanged(this, InQuickBar);
}

int32 AFortQuickBars::TrySetFocused(EFortQuickBars InQuickBar, int32 Slot, bool bUpdatePreviousFocusedSlot)
{
    static auto TrySetFocused = Scanner::Function<int32 (*)(AFortQuickBars*, EFortQuickBars, int32, bool)>(Scanner::At(GetActivateSlot(this)).Find("call 0x*").Target(TEXT("AFortQuickBars::TrySetFocused")));

    return TrySetFocused(this, InQuickBar, Slot, bUpdatePreviousFocusedSlot);
}

void AFortQuickBars::ServerRemoveItemInternal_Implementation(const FGuid& Item, bool bFindReplacement, bool bForce)
{
    for (auto QuickBarType : { EFortQuickBars::Primary, EFortQuickBars::Secondary })
    {
        for (auto& Slot : GetQuickBar(QuickBarType).Slots)
        {
            if (Slot.Items.RemoveSingle(Item))
            {
                OnQuickBarChanged(QuickBarType);
                return;
            }
        }
    }
}
