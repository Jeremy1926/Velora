#include "pch.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

static auto IsNetRelevantForSlot = []
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Actor %s / %s has no root component in AActor::IsNetRelevantFor. (Make bAlwaysRelevant=true?)")));
    auto End = Scanner::FindFunctionEnd(Function);

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
            return (int32_t)Instruction.disp.disp32;

        Address += Length;
    }

    return 0;
}();

static int32_t GetNetPrioritySlot()
{
    static auto GetNetPriority = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));

        for (auto Index = 0; Index * (int32_t)sizeof(void*) < IsNetRelevantForSlot; Index++)
        {
            auto Function = VTable[Index];

            if (!memcmp(Function, "\xF3\x0F\x10\x05", 4) && Function[8] == 0xC3 && *(float*)(Function + 8 + *(int32_t*)(Function + 4)) == -17.0f)
                return (Index + 1) * (int32_t)sizeof(void*);
        }

        return 0;
    }();

    return GetNetPriority;
}

bool AActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AActor*, const AActor*, const AActor*, const FVector&))(VTable + IsNetRelevantForSlot))(this, RealViewer, ViewTarget, SrcLocation);
}

float AActor::GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    auto VTable = *(uint8_t**)this;

    return (*(float (**)(AActor*, const FVector&, const FVector&, AActor*, AActor*, UActorChannel*, float, bool))(VTable + GetNetPrioritySlot()))(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

bool AActor::GetNetDormancy(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    static auto GetNetDormancy = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));

        for (auto Index = GetNetPrioritySlot() / (int32_t)sizeof(void*); Index * (int32_t)sizeof(void*) < IsNetRelevantForSlot; Index++)
        {
            auto Function = Scanner::FindFunction(VTable[Index]);

            if (!Function)
                continue;

            auto End = Scanner::FindFunctionEnd(Function);

            for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
            {
                hde64s Instruction;
                auto Length = hde64_disasm(Address, &Instruction);

                if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2 && (int32_t)Instruction.disp.disp32 == (Index + 1) * (int32_t)sizeof(void*))
                    return (int32_t)Instruction.disp.disp32;

                Address += Length;
            }
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(AActor*, const FVector&, const FVector&, AActor*, AActor*, UActorChannel*, float, bool))(VTable + GetNetDormancy))(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

void AActor::CallPreReplication(UNetDriver* NetDriver)
{
    static auto CallPreReplication = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Actor %s took more than 95%% of maximum desired recording time. Actor: %.3fms. Max: %.3fms."));
        auto Function = Scanner::FindFunction(Reference);
        const uint8_t* Call = nullptr;
        const uint8_t* PreReplication = nullptr;
        bool bReplicateActor = false;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                if (bReplicateActor)
                    PreReplication = Call;

                Call = Address + Length + (int32_t)Instruction.imm.imm32;
                bReplicateActor = false;
            }
            else if (Instruction.opcode == 0x33 && Instruction.rex_r && Instruction.rex_b && Instruction.modrm == 0xC9)
                bReplicateActor = true;

            Address += Length;
        }

        return (void (*)(AActor*, UNetDriver*))PreReplication;
    }();

    CallPreReplication(this, NetDriver);
}
