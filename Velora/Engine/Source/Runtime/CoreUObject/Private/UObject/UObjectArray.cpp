#include "pch.h"
#include "UObject/UObjectArray.h"
#include "Scanner.h"
#include "hde/hde64.h"

FUObjectArray& GUObjectArray = *[]
{
    auto Reference = Scanner::FindStringReference(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "));
    auto Function = Scanner::FindFunction(Reference);
    const uint8_t* NumElements = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);
        auto Target = Address + Length + (int32_t)Instruction.disp.disp32;

        if (Instruction.opcode == 0x3B && (Instruction.modrm & 0xC7) == 0x05)
            NumElements = Target;
        else if (Instruction.opcode == 0x8B && Instruction.rex_w && (Instruction.modrm & 0xC7) == 0x05 && Target + offsetof(FFixedUObjectArray, NumElements) == NumElements)
            return (FUObjectArray*)(Target - offsetof(FUObjectArray, ObjObjects));

        Address += Length;
    }

    return (FUObjectArray*)nullptr;
}();
