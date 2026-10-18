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

void AFortQuickBars::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    static auto ActivateSlot = (void (*)(AFortQuickBars*, EFortQuickBars, int32, float, bool))Scanner::String(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d.")).Find("mov byte ptr [rsp+0x20], 0x1; call 0x*").Call().Get();

    ActivateSlot(this, InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot);
}

FQuickBar& AFortQuickBars::GetQuickBar(EFortQuickBars InQuickBar)
{
    static auto PrimaryQuickBar = GetClass()->FindPropertyByName(FName(TEXT("PrimaryQuickBar")));
    static auto SecondaryQuickBar = GetClass()->FindPropertyByName(FName(TEXT("SecondaryQuickBar")));

    return *(InQuickBar == EFortQuickBars::Secondary ? SecondaryQuickBar : PrimaryQuickBar)->ContainerPtrToValuePtr<FQuickBar>(this);
}

void AFortQuickBars::OnQuickBarChanged(EFortQuickBars InQuickBar)
{
    static auto OnQuickBarChanged = (void (*)(AFortQuickBars*, EFortQuickBars))Scanner::At(Scanner::Exec(TEXT("/Script/FortniteGame.FortQuickBars:OnRep_SecondaryQuickBar")).Find("jmp 0x*").Target()).Find("jmp 0x*").Target();

    OnQuickBarChanged(this, InQuickBar);
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
