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
    static auto ActivateSlot = Anchor::Optional<void (*)(AFortQuickBars*, EFortQuickBars, int32, float, bool, bool, bool)>(TEXT("AFortQuickBars::ActivateSlot"))
        .String(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d."))
        .Find("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    if (ActivateSlot)
        return ActivateSlot(this, InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot, false, false);

    static auto VirtualActivateSlot = Anchor::VirtualFunction<void (*)(AFortQuickBars*, EFortQuickBars, int32, float, bool, bool, bool)>(TEXT("AFortQuickBars::ActivateSlot"))
        .String(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d."))
        .Find("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .Get();

    VirtualActivateSlot(this, InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot, false, false);
}

FQuickBar& AFortQuickBars::GetQuickBar(EFortQuickBars InQuickBar)
{
    static auto PrimaryQuickBar = GetClass()->FindPropertyByName(FName(TEXT("PrimaryQuickBar")));
    static auto SecondaryQuickBar = GetClass()->FindPropertyByName(FName(TEXT("SecondaryQuickBar")));

    return *(InQuickBar == EFortQuickBars::Secondary ? SecondaryQuickBar : PrimaryQuickBar)->ContainerPtrToValuePtr<FQuickBar>(this);
}

void AFortQuickBars::OnQuickBarChanged(EFortQuickBars InQuickBar)
{
    static auto OnQuickBarChanged = Anchor::Function<void (*)(AFortQuickBars*, EFortQuickBars, FQuickBar*, FQuickBar*)>(TEXT("AFortQuickBars::OnQuickBarChanged"))
        .Exec(TEXT("/Script/FortniteGame.FortQuickBars:OnRep_SecondaryQuickBar"))
        .Find("jmp 0x*")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Get();
    static auto PrimaryQuickBar_Previous = GetClass()->FindPropertyByName(FName(TEXT("PrimaryQuickBar_Previous")));
    static auto SecondaryQuickBar_Previous = GetClass()->FindPropertyByName(FName(TEXT("SecondaryQuickBar_Previous")));

    auto QuickBar_Previous = InQuickBar == EFortQuickBars::Secondary ? SecondaryQuickBar_Previous : PrimaryQuickBar_Previous;

    OnQuickBarChanged(this, InQuickBar, &GetQuickBar(InQuickBar), QuickBar_Previous ? QuickBar_Previous->ContainerPtrToValuePtr<FQuickBar>(this) : nullptr);
}

int32 AFortQuickBars::TrySetFocused(EFortQuickBars InQuickBar, int32 Slot, bool bUpdatePreviousFocusedSlot)
{
    static auto TrySetFocused = Anchor::Function<int32 (*)(AFortQuickBars*, EFortQuickBars, int32, bool)>(TEXT("AFortQuickBars::TrySetFocused"))
        .String(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d."))
        .Find("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Log(TEXT("AFortQuickBars::ActivateSlot"))
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d."))
        .Find("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .LogSlot(TEXT("AFortQuickBars::ActivateSlot"))
        .Entry(this)
        .Find("call 0x*")
        .Target()
        .Get();

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
