#include "pch.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

UClass* AGameModeBase::GetDefaultPawnClassForController(AController* InController)
{
    static auto GetDefaultPawnClassForController = []
    {
        for (auto Address = Scanner::FindStringReference(TEXT("SpawnDefaultPawnFor: Failed due to no start spot"));;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                return (UClass* (*)(AGameModeBase*, AController*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    return GetDefaultPawnClassForController(this, InController);
}

APawn* AGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    static auto SpawnDefaultPawnFor = []
    {
        auto AthenaSpawnDefaultPawnFor = Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("SpawnDefaultPawnFor: Failed due to no world")))->BeginAddress;
        auto AthenaVTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (AthenaVTable[Index] != AthenaSpawnDefaultPawnFor)
            Index++;

        auto VTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__GameModeBase"));

        return (APawn* (*)(AGameModeBase*, AController*, AActor*))VTable[Index];
    }();

    return SpawnDefaultPawnFor(this, NewPlayer, StartSpot);
}
