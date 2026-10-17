#include "pch.h"
#include "Engine/NetDriver.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "hde/hde64.h"

int32 UNetDriver::GetMaxInternetClientRate() const
{
    static auto MaxInternetClientRate = GetClass()->FindPropertyByName(FName(TEXT("MaxInternetClientRate")));

    return *MaxInternetClientRate->ContainerPtrToValuePtr<int32>(this);
}

int32 UNetDriver::GetMaxClientRate() const
{
    static auto MaxClientRate = GetClass()->FindPropertyByName(FName(TEXT("MaxClientRate")));

    return *MaxClientRate->ContainerPtrToValuePtr<int32>(this);
}

void UNetDriver::SetMaxClientRate(int32 InMaxClientRate)
{
    static auto MaxClientRate = GetClass()->FindPropertyByName(FName(TEXT("MaxClientRate")));

    *MaxClientRate->ContainerPtrToValuePtr<int32>(this) = InMaxClientRate;
}

float UNetDriver::GetServerTravelPause() const
{
    static auto ServerTravelPause = GetClass()->FindPropertyByName(FName(TEXT("ServerTravelPause")));

    return *ServerTravelPause->ContainerPtrToValuePtr<float>(this);
}

bool UNetDriver::InitListen(FNetworkNotify* InNotify, FURL& ListenURL, bool bReuseAddressAndPort, FString& Error)
{
    static auto InitListen = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(UNetDriver*, FNetworkNotify*, FURL&, bool, FString&))(VTable + InitListen))(this, InNotify, ListenURL, bReuseAddressAndPort, Error);
}

void UNetDriver::SetWorld(UWorld* InWorld)
{
    static auto SetWorld = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        const uint8_t* Call = nullptr;
        bool bDynamicSourceLevels = false;

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                if (bDynamicSourceLevels)
                    return (void (*)(UNetDriver*, UWorld*))Call;

                Call = Address + Length + (int32_t)Instruction.imm.imm32;
            }
            else if (Instruction.opcode == 0x33 && Instruction.modrm == 0xD2)
                bDynamicSourceLevels = true;

            Address += Length;
        }

        return (void (*)(UNetDriver*, UWorld*))nullptr;
    }();

    SetWorld(this, InWorld);
}
