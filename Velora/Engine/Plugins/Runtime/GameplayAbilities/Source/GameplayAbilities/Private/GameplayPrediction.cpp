#include "pch.h"
#include "GameplayPrediction.h"
#include "Scanner.h"
#include "hde/hde64.h"

FScopedPredictionWindow::FScopedPredictionWindow(UAbilitySystemComponent* AbilitySystemComponent, FPredictionKey InPredictionKey, bool InSetReplicatedPredictionKey)
{
    static auto Constructor = []
    {
        for (auto Address = (const uint8_t*)Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Ability %s is overriding pending replicated target data.")))->BeginAddress;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                return (void (*)(FScopedPredictionWindow*, UAbilitySystemComponent*, FPredictionKey*, bool))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    Constructor(this, AbilitySystemComponent, &InPredictionKey, InSetReplicatedPredictionKey);
}

FScopedPredictionWindow::~FScopedPredictionWindow()
{
    static auto Destructor = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Ability %s is overriding pending replicated target data.")));
        auto End = Scanner::FindFunctionEnd(Function);
        const uint8_t* Call = nullptr;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (void (*)(FScopedPredictionWindow*))Call;
    }();

    Destructor(this);
}
