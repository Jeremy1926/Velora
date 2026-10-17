#include "pch.h"
#include "Items/FortQuickBars.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

UClass* AFortQuickBars::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuickBars"));

    return Class;
}

void AFortQuickBars::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    static auto ActivateSlot = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Attempting to add harvesting tool %s to quickbar %d in slot %d."));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        auto bUpdatePreviousFocusedSlot = false;

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && bUpdatePreviousFocusedSlot)
                return (void (*)(AFortQuickBars*, EFortQuickBars, int32, float, bool))(Address + Length + (int32_t)Instruction.imm.imm32);

            bUpdatePreviousFocusedSlot = !memcmp(Address, "\xC6\x44\x24\x20\x01", 5);
            Address += Length;
        }

        return (void (*)(AFortQuickBars*, EFortQuickBars, int32, float, bool))nullptr;
    }();

    ActivateSlot(this, InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot);
}
