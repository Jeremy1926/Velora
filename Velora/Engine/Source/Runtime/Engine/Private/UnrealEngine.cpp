#include "pch.h"
#include "Engine/Engine.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "hde/hde64.h"

UEngine*& GEngine = *[]
{
    auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
    auto Function = Scanner::FindFunction(Reference);
    UEngine** Engine = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x8B && Instruction.rex_w && Instruction.modrm == 0x0D)
            Engine = (UEngine**)(Address + Length + (int32_t)Instruction.disp.disp32);

        Address += Length;
    }

    return Engine;
}();

static auto NetworkFailureEvent = []
{
    auto Reference = Scanner::FindStringReference(TEXT("Server switch level: %s"));
    auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
    int32_t Offset = 0;
    bool bPendingConnectionFailure = false;

    for (auto Address = Reference; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8 && bPendingConnectionFailure)
            return std::pair(Offset, Address + Length + (int32_t)Instruction.imm.imm32);
        else if (Instruction.opcode == 0x8D && Instruction.modrm_reg == 1 && Instruction.modrm_mod == 2)
            Offset = (int32_t)Instruction.disp.disp32;
        else if (Instruction.opcode == 0xB9 && Instruction.rex_b && Instruction.imm.imm32 == ENetworkFailure::PendingConnectionFailure)
            bPendingConnectionFailure = true;

        Address += Length;
    }

    return std::pair<int32_t, const uint8_t*>();
}();

void FWorldContext::SetRunAsDedicated(bool bInRunAsDedicated)
{
    static auto RunAsDedicated = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("TeamChatroom")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && !memcmp(Address + Length, "\x84\xC0\x0F\x85", 4))
            {
                for (auto Body = Address + Length + (int32_t)Instruction.imm.imm32; Instruction.opcode != 0xC3;)
                {
                    Body += hde64_disasm(Body, &Instruction);

                    if (Instruction.opcode == 0x0F && Instruction.opcode2 == 0xB6 && Instruction.modrm_mod == 2)
                        return (int32_t)Instruction.disp.disp32;
                }
            }

            Address += Length;
        }

        return 0;
    }();

    *(bool*)((uint8_t*)this + RunAsDedicated) = bInRunAsDedicated;
}

float UEngine::GetNetClientTicksPerSecond() const
{
    static auto NetClientTicksPerSecond = GetClass()->FindPropertyByName(FName(TEXT("NetClientTicksPerSecond")));

    return *NetClientTicksPerSecond->ContainerPtrToValuePtr<float>(this);
}

float UEngine::GetMaxTickRate(float DeltaTime, bool bAllowFrameRateSmoothing) const
{
    static auto GetMaxTickRate = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Bringing %s up for play (max tick rate %i) at %s"));
        auto Function = Scanner::FindFunction(Reference);
        int32_t Slot = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                Slot = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return Slot;
    }();

    auto VTable = *(uint8_t**)this;

    return (*(float (**)(const UEngine*, float, bool))(VTable + GetMaxTickRate))(this, DeltaTime, bAllowFrameRateSmoothing);
}

UGameViewportClient* UEngine::GetGameViewport() const
{
    static auto GameViewport = GetClass()->FindPropertyByName(FName(TEXT("GameViewport")));

    return *GameViewport->ContainerPtrToValuePtr<UGameViewportClient*>(this);
}

UClass* UEngine::GetConsoleClass() const
{
    static auto ConsoleClass = GetClass()->FindPropertyByName(FName(TEXT("ConsoleClass")));

    return *ConsoleClass->ContainerPtrToValuePtr<UClass*>(this);
}

UEngine::FOnNetworkFailure& UEngine::OnNetworkFailure()
{
    return *(FOnNetworkFailure*)((uint8_t*)this + NetworkFailureEvent.first);
}

void UEngine::FOnNetworkFailure::Broadcast(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString) const
{
    static auto Broadcast = (void (*)(const FOnNetworkFailure*, UWorld*, UNetDriver*, ENetworkFailure::Type, const FString&))NetworkFailureEvent.second;

    Broadcast(this, World, NetDriver, FailureType, ErrorString);
}

bool UEngine::CreateNamedNetDriver(UWorld* InWorld, FName NetDriverName, FName NetDriverDefinition)
{
    static auto CreateNamedNetDriver = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto Function = Scanner::FindFunction(Reference);
        const uint8_t* Call = nullptr;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (bool (*)(UEngine*, UWorld*, FName, FName))Call;
    }();

    return CreateNamedNetDriver(this, InWorld, NetDriverName, NetDriverDefinition);
}

UNetDriver* UEngine::FindNamedNetDriver(UWorld* InWorld, FName NetDriverName)
{
    static auto FindNamedNetDriver = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        bool bEngine = false;

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && bEngine)
                return (UNetDriver* (*)(UEngine*, UWorld*, FName))(Address + Length + (int32_t)Instruction.imm.imm32);

            bEngine = Instruction.opcode == 0x8B && Instruction.rex_w && Instruction.modrm == 0x0D && Address + Length + (int32_t)Instruction.disp.disp32 == (const uint8_t*)&GEngine;
            Address += Length;
        }

        return (UNetDriver* (*)(UEngine*, UWorld*, FName))nullptr;
    }();

    return FindNamedNetDriver(this, InWorld, NetDriverName);
}
