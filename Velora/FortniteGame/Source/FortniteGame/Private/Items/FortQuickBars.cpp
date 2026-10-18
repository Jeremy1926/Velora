#include "pch.h"
#include "Items/FortQuickBars.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortQuickBars::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuickBars"));

    return Class;
}

void AFortQuickBars::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    static auto ActivateSlot = (void (*)(AFortQuickBars*, EFortQuickBars, int32, float, bool))Scanner::String(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d.")).Find("mov byte ptr [rsp+0x20], 0x1; call 0x*").Call().Get();

    ActivateSlot(this, InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot);
}
