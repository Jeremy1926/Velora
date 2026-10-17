#include "pch.h"
#include "Engine/GameInstance.h"
#include "Scanner.h"
#include "hde/hde64.h"

bool UGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
    static auto RemoveLocalPlayer = (bool (*)(UGameInstance*, ULocalPlayer*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("UGameInstance::RemovePlayer: Removed player %s with ControllerId %i at index %i (%i remaining players)")))->BeginAddress);

    return RemoveLocalPlayer(this, ExistingPlayer);
}

ULocalPlayer* UGameInstance::GetFirstGamePlayer() const
{
    static auto GetFirstGamePlayer = (ULocalPlayer* (*)(const UGameInstance*))Scanner::FindPattern("83 79 40 00 7E ? 48 8B 41 38 48 8B 00 C3 33 C0 C3");

    return GetFirstGamePlayer(this);
}

FWorldContext* UGameInstance::GetWorldContext() const
{
    static auto WorldContext = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("TeamChatroom")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && !memcmp(Address + Length, "\x84\xC0\x0F\x85", 4))
            {
                hde64s IsDedicatedServerInstance;
                hde64_disasm(Address + Length + (int32_t)Instruction.imm.imm32, &IsDedicatedServerInstance);

                return (int32_t)IsDedicatedServerInstance.disp.disp8;
            }

            Address += Length;
        }

        return 0;
    }();

    return *(FWorldContext**)((uint8_t*)this + WorldContext);
}
