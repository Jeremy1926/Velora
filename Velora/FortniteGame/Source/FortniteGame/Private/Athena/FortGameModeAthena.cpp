#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Templates/Casts.h"
#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static FLogCategoryBase& LogFort = *[]
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("SpawnDefaultPawnFor: Failed due to no world")));

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress;;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x80 && Instruction.modrm_reg == 7 && (Instruction.modrm & 0xC7) == 0x05)
            return (FLogCategoryBase*)(Address + Length + (int32_t)Instruction.disp.disp32);

        Address += Length;
    }
}();

static auto SpawnDefaultPawnFor = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("SpawnDefaultPawnFor: Failed due to no world")))->BeginAddress, &AFortGameModeAthena::SpawnDefaultPawnFor_Implementation);

UClass* AFortGameModeAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));

    return Class;
}

APawn* AFortGameModeAthena::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    if (!Cast<AFortPlayerControllerAthena>(NewPlayer))
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no player controller"));
        return nullptr;
    }

    if (!StartSpot)
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no start spot"));
        return nullptr;
    }

    if (!GetDefaultPawnClassForController(NewPlayer))
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no default pawn class"));
        return nullptr;
    }

    if (!GetWorld())
    {
        UE_LOG(LogFort, Warning, TEXT("SpawnDefaultPawnFor: Failed due to no world"));
        return nullptr;
    }

    return AGameModeBase::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}
