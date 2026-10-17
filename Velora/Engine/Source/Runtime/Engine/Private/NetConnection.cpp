#include "pch.h"
#include "Engine/NetConnection.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

static auto RemoteFunctionChecks = []
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it")));
    auto End = Scanner::FindFunctionEnd(Function);
    std::pair<int32_t, int32_t> Checks = {};
    bool bSaturate = false;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End && (!Checks.first || !Checks.second);)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x33 && Instruction.modrm == 0xD2)
            bSaturate = true;
        else if (bSaturate && !Checks.first && Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
            Checks.first = (int32_t)Instruction.disp.disp32;
        else if (!Checks.second && Instruction.opcode == 0x83 && Instruction.modrm_reg == 7 && Instruction.modrm_mod == 2 && Instruction.imm.imm8 == USOCK_Closed)
            Checks.second = (int32_t)Instruction.disp.disp32;

        Address += Length;
    }

    return Checks;
}();

TArray<UChildConnection*>& UNetConnection::GetChildren()
{
    static auto Children = GetClass()->FindPropertyByName(FName(TEXT("Children")));

    return *Children->ContainerPtrToValuePtr<TArray<UChildConnection*>>(this);
}

UNetDriver* UNetConnection::GetDriver() const
{
    static auto Driver = GetClass()->FindPropertyByName(FName(TEXT("Driver")));

    return *Driver->ContainerPtrToValuePtr<UNetDriver*>(this);
}

UPackageMap* UNetConnection::GetPackageMap() const
{
    static auto PackageMap = GetClass()->FindPropertyByName(FName(TEXT("PackageMap")));

    return *PackageMap->ContainerPtrToValuePtr<UPackageMap*>(this);
}

TArray<AActor*>& UNetConnection::GetSentTemporaries()
{
    static auto SentTemporaries = GetClass()->FindPropertyByName(FName(TEXT("SentTemporaries")));

    return *SentTemporaries->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

AActor* UNetConnection::GetViewTarget() const
{
    static auto ViewTarget = GetClass()->FindPropertyByName(FName(TEXT("ViewTarget")));

    return *ViewTarget->ContainerPtrToValuePtr<AActor*>(this);
}

void UNetConnection::SetViewTarget(AActor* InViewTarget)
{
    static auto ViewTarget = GetClass()->FindPropertyByName(FName(TEXT("ViewTarget")));

    *ViewTarget->ContainerPtrToValuePtr<AActor*>(this) = InViewTarget;
}

AActor* UNetConnection::GetOwningActor() const
{
    static auto OwningActor = GetClass()->FindPropertyByName(FName(TEXT("OwningActor")));

    return *OwningActor->ContainerPtrToValuePtr<AActor*>(this);
}

bool UNetConnection::GetInternalAck() const
{
    static auto InternalAck = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("InternalAck")));

    return InternalAck->GetPropertyValue_InContainer(this);
}

EConnectionState UNetConnection::GetState() const
{
    return *(EConnectionState*)((uint8_t*)this + RemoteFunctionChecks.second);
}

double UNetConnection::GetLastReceiveTime() const
{
    static auto LastReceiveTime = GetClass()->FindPropertyByName(FName(TEXT("LastReceiveTime")));

    return *LastReceiveTime->ContainerPtrToValuePtr<double>(this);
}

static int32_t TickCountOffset()
{
    static auto TickCount = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__NetConnection"));

        for (auto Address = VTable[RemoteFunctionChecks.first / sizeof(void*)];;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x89 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32 + (int32_t)sizeof(int32);

            Address += Length;
        }
    }();

    return TickCount;
}

int32 UNetConnection::GetTickCount() const
{
    return *(int32*)((uint8_t*)this + TickCountOffset());
}

void UNetConnection::SetTickCount(int32 InTickCount)
{
    *(int32*)((uint8_t*)this + TickCountOffset()) = InTickCount;
}

void UNetConnection::SetTimeSensitive(bool bInTimeSensitive)
{
    static auto TimeSensitive = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UNetConnection::Tick: Connection TIMED OUT. Closing connection.")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x80 && Instruction.modrm_reg == 7 && Instruction.modrm_mod == 2 && Instruction.imm.imm8 == 0)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    *(bool*)((uint8_t*)this + TimeSensitive) = bInTimeSensitive;
}

TMap<TWeakObjectPtr<AActor>, UActorChannel*, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, UActorChannel*>>& UNetConnection::GetActorChannels()
{
    static auto ActorChannels = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x81 && Instruction.rex_w && Instruction.modrm == 0xC1)
                return (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return 0;
    }();

    return *(TMap<TWeakObjectPtr<AActor>, UActorChannel*, TWeakObjectPtrMapKeyFuncs<TWeakObjectPtr<AActor>, UActorChannel*>>*)((uint8_t*)this + ActorChannels);
}

TSet<FNetworkGUID>& UNetConnection::GetDestroyedStartupOrDormantActors()
{
    static auto DestroyedStartupOrDormantActors = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("AddClientConnection: Added client connection: %s"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x8D && Instruction.rex_w && !Instruction.rex_r && Instruction.modrm_reg == 1 && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        return 0;
    }();

    return *(TSet<FNetworkGUID>*)((uint8_t*)this + DestroyedStartupOrDormantActors);
}

TSet<FName>& UNetConnection::GetClientVisibleLevelNames()
{
    static auto ClientVisibleLevelNames = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("ServerUpdateLevelVisibility() Removed '%s'"));
        auto Function = Scanner::FindFunction(Reference);
        int32_t Offset = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x81 && Instruction.rex_w && Instruction.modrm == 0xC1)
                Offset = (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return Offset;
    }();

    return *(TSet<FName>*)((uint8_t*)this + ClientVisibleLevelNames);
}

int32 UNetConnection::IsNetReady(bool Saturate)
{
    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(UNetConnection*, bool))(VTable + RemoteFunctionChecks.first))(this, Saturate);
}

UChannel* UNetConnection::CreateChannel(EChannelType Type, bool bOpenedLocally, int32 ChannelIndex)
{
    static auto CreateChannel = (UChannel* (*)(UNetConnection*, EChannelType, bool, int32))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Created channel %i of type %i")))->BeginAddress);

    return CreateChannel(this, Type, bOpenedLocally, ChannelIndex);
}

void UNetConnection::FlushDormancy(AActor* Actor)
{
    static auto FlushDormancy = []
    {
        auto Function = Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("FlushDormancy: %s. Connection: %s")))->BeginAddress;
        auto VTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__NetConnection"));
        auto Index = 0;

        while (VTable[Index] != Function)
            Index++;

        return Index;
    }();

    auto VTable = *(void (***)(UNetConnection*, AActor*))this;

    VTable[FlushDormancy](this, Actor);
}
