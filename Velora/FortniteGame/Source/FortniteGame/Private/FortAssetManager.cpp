#include "pch.h"
#include "FortAssetManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "hde/hde64.h"

static const uint8_t* FindGetPersonalXpBoostCall(int32 Index)
{
    const uint8_t* GetPersonalXpBoost = nullptr;

    for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost")))->GetNativeFunc()); !GetPersonalXpBoost;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
            GetPersonalXpBoost = Address + Length + (int32_t)Instruction.imm.imm32;

        Address += Length;
    }

    for (auto Calls = 0;;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(GetPersonalXpBoost, &Instruction);

        if (Instruction.opcode == 0xE8 && Calls++ == Index)
            return GetPersonalXpBoost + Length + (int32_t)Instruction.imm.imm32;

        GetPersonalXpBoost += Length;
    }
}

UFortAssetManager& UFortAssetManager::Get()
{
    static auto Get = (UFortAssetManager& (*)())FindGetPersonalXpBoostCall(0);

    return Get();
}

UFortGameData* UFortAssetManager::GetGameData()
{
    static auto GetGameData = (UFortGameData* (*)(UFortAssetManager*))FindGetPersonalXpBoostCall(1);

    return GetGameData(this);
}
