#include "pch.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

void UGameplayStatics::OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute, FString Options)
{
    static auto OpenLevel = (void (*)(const UObject*, FName, bool, FString))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("WARNING: The map '%s' does not exist.")))->BeginAddress);

    OpenLevel(WorldContextObject, LevelName, bAbsolute, MoveTemp(Options));
}

void UGameplayStatics::GetAllActorsOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors)
{
    static auto GetAllActorsOfClass = []
    {
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.GameplayStatics:GetAllActorsOfClass")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                return (void (*)(const UObject*, TSubclassOf<AActor>, TArray<AActor*>&))(Address + Length + (int32_t)Instruction.imm.imm32);

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }
    }();

    GetAllActorsOfClass(WorldContextObject, ActorClass, OutActors);
}
