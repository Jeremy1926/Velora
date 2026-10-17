#include "pch.h"
#include "Components/SceneComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

const FTransform& USceneComponent::GetComponentTransform() const
{
    static auto ComponentToWorld = []
    {
        auto Thunk = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Actor:K2_GetActorLocation")))->GetNativeFunc());
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Thunk));

        for (auto Address = Thunk; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x0F && Instruction.opcode2 == 0x28 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32 - (int32_t)sizeof(VectorRegister);

            Address += Length;
        }

        return 0;
    }();

    return *(FTransform*)((uint8_t*)this + ComponentToWorld);
}
