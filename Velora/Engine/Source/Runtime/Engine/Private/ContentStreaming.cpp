#include "pch.h"
#include "ContentStreaming.h"
#include "Scanner.h"
#include "hde/hde64.h"

static auto StreamingViewCalls = []
{
    auto Reference = Scanner::FindStringReference(TEXT("DemoRec"));
    auto Function = Scanner::FindFunction(Reference);
    std::array<const uint8_t*, 2> Calls = {};
    const uint8_t* Call = nullptr;
    bool bRemoveStreamingViewsAll = false;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
        {
            auto Target = Address + Length + (int32_t)Instruction.imm.imm32;

            if (bRemoveStreamingViewsAll)
                Calls = { Call, Target };

            Call = Target;
        }

        bRemoveStreamingViewsAll = Instruction.opcode == 0xBA && !Instruction.rex_b && Instruction.imm.imm32 == RemoveStreamingViews_All;
        Address += Length;
    }

    return Calls;
}();

FStreamingManagerCollection& IStreamingManager::Get()
{
    static auto Get = (FStreamingManagerCollection& (*)())StreamingViewCalls[0];

    return Get();
}

void IStreamingManager::RemoveStreamingViews(ERemoveStreamingViews RemovalType)
{
    static auto RemoveStreamingViews = (void (*)(IStreamingManager*, ERemoveStreamingViews))StreamingViewCalls[1];

    RemoveStreamingViews(this, RemovalType);
}
