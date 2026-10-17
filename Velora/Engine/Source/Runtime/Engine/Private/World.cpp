#include "pch.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Logging/LogMacros.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static FLogCategoryBase& LogWorld = *[]
{
    auto Reference = Scanner::FindStringReference(TEXT("Bringing %s up for play (max tick rate %i) at %s"));
    auto Function = Scanner::FindFunction(Reference);
    FLogCategoryBase* Category = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x0F && Instruction.opcode2 == 0xB6 && (Instruction.modrm & 0xC7) == 0x05)
            Category = (FLogCategoryBase*)(Address + Length + (int32_t)Instruction.disp.disp32);

        Address += Length;
    }

    return Category;
}();

static auto Listen = DetourCall([]
{
    auto Reference = Scanner::FindStringReference(TEXT("LoadMap: failed to Listen(%s)"));
    auto Function = Scanner::FindFunction(Reference);
    const uint8_t* Call = nullptr;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8 && !memcmp(Address + Length, "\x84\xC0", 2))
            Call = Address;

        Address += Length;
    }

    return Call;
}(), &UWorld::Listen);

AGameNetworkManager* UWorld::GetNetworkManager() const
{
    static auto NetworkManager = GetClass()->FindPropertyByName(FName(TEXT("NetworkManager")));

    return *NetworkManager->ContainerPtrToValuePtr<AGameNetworkManager*>(this);
}

ULevel* UWorld::GetCurrentLevelPendingVisibility() const
{
    static auto CurrentLevelPendingVisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingVisibility")));

    return *CurrentLevelPendingVisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

ULevel* UWorld::GetCurrentLevelPendingInvisibility() const
{
    static auto CurrentLevelPendingInvisibility = GetClass()->FindPropertyByName(FName(TEXT("CurrentLevelPendingInvisibility")));

    return *CurrentLevelPendingInvisibility->ContainerPtrToValuePtr<ULevel*>(this);
}

float UWorld::GetTimeSeconds() const
{
    static auto TimeSeconds = []
    {
        auto Thunk = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.GameplayStatics:GetTimeSeconds")))->GetNativeFunc());
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Thunk));
        const uint8_t* Call = nullptr;

        for (auto Address = Thunk; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        for (auto Address = Call;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x0F && Instruction.opcode2 == 0x10 && Instruction.p_rep && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }
    }();

    return *(float*)((uint8_t*)this + TimeSeconds);
}

AWorldSettings* UWorld::GetWorldSettings(bool bCheckStreamingPesistent, bool bChecked) const
{
    static auto GetWorldSettings = []
    {
        auto Thunk = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.GameplayStatics:GetGlobalTimeDilation")))->GetNativeFunc());
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Thunk));
        const uint8_t* Call = nullptr;

        for (auto Address = Thunk; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        auto ImplEnd = Scanner::FindFunctionEnd(Scanner::FindFunction(Call));

        for (auto Address = Call; Address < ImplEnd;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (AWorldSettings* (*)(const UWorld*, bool, bool))Call;
    }();

    return GetWorldSettings(this, bCheckStreamingPesistent, bChecked);
}

UNetDriver* UWorld::GetNetDriver() const
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    return *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

void UWorld::SetNetDriver(UNetDriver* NewDriver)
{
    static auto NetDriver = GetClass()->FindPropertyByName(FName(TEXT("NetDriver")));

    *NetDriver->ContainerPtrToValuePtr<UNetDriver*>(this) = NewDriver;
}

void UWorld::SetNextSwitchCountdown(float InNextSwitchCountdown)
{
    static auto NextSwitchCountdown = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Server switch level: %s"));
        auto Function = Scanner::FindFunction(Reference);
        bool bSubtract = false;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x0F && Instruction.opcode2 == 0x5C)
                bSubtract = true;
            else if (bSubtract && Instruction.opcode == 0x0F && Instruction.opcode2 == 0x11 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    *(float*)((uint8_t*)this + NextSwitchCountdown) = InNextSwitchCountdown;
}

FLevelCollection* UWorld::FindCollectionByType(const ELevelCollectionType InType)
{
    static auto FindCollectionByType = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("RecordReplay: failed to create demo net driver!"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
        bool bDynamicSourceLevels = false;

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && bDynamicSourceLevels)
                return (FLevelCollection* (*)(UWorld*, ELevelCollectionType))(Address + Length + (int32_t)Instruction.imm.imm32);
            else if (Instruction.opcode == 0x33 && Instruction.modrm == 0xD2)
                bDynamicSourceLevels = true;

            Address += Length;
        }

        return (FLevelCollection* (*)(UWorld*, ELevelCollectionType))nullptr;
    }();

    return FindCollectionByType(this, InType);
}

bool UWorld::Listen(FURL& InURL)
{
    if (GetNetDriver())
    {
        GEngine->BroadcastNetworkFailure(this, GetNetDriver(), ENetworkFailure::NetDriverAlreadyExists);
        return false;
    }

    if (GEngine->CreateNamedNetDriver(this, NAME_GameNetDriver, NAME_GameNetDriver))
    {
        SetNetDriver(GEngine->FindNamedNetDriver(this, NAME_GameNetDriver));
        GetNetDriver()->SetWorld(this);

        FLevelCollection* const SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);

        if (SourceCollection)
            SourceCollection->SetNetDriver(GetNetDriver());

        FLevelCollection* const StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);

        if (StaticCollection)
            StaticCollection->SetNetDriver(GetNetDriver());
    }

    if (GetNetDriver() == nullptr)
    {
        GEngine->BroadcastNetworkFailure(this, nullptr, ENetworkFailure::NetDriverCreateFailure);
        return false;
    }

    FString Error;

    if (!GetNetDriver()->InitListen(this, InURL, false, Error))
    {
        GEngine->BroadcastNetworkFailure(this, GetNetDriver(), ENetworkFailure::NetDriverListenFailure, Error);
        UE_LOG(LogWorld, Log, TEXT("Failed to listen: %s"), *Error);
        GetNetDriver()->SetWorld(nullptr);
        SetNetDriver(nullptr);

        FLevelCollection* SourceCollection = FindCollectionByType(ELevelCollectionType::DynamicSourceLevels);

        if (SourceCollection)
            SourceCollection->SetNetDriver(nullptr);

        FLevelCollection* StaticCollection = FindCollectionByType(ELevelCollectionType::StaticLevels);

        if (StaticCollection)
            StaticCollection->SetNetDriver(nullptr);

        return false;
    }

    static bool LanPlay = FParse::Param(FCommandLine::Get(), TEXT("lanplay"));

    if (!LanPlay && (GetNetDriver()->GetMaxInternetClientRate() < GetNetDriver()->GetMaxClientRate()) && (GetNetDriver()->GetMaxInternetClientRate() > 2500))
        GetNetDriver()->SetMaxClientRate(GetNetDriver()->GetMaxInternetClientRate());

    SetNextSwitchCountdown(GetNetDriver()->GetServerTravelPause());
    return true;
}
