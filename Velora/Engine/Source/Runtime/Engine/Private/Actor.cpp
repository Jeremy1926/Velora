#include "pch.h"
#include "GameFramework/Actor.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

bool AActor::GetOnlyRelevantToOwner() const
{
    static auto bOnlyRelevantToOwner = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bOnlyRelevantToOwner")));

    return bOnlyRelevantToOwner->GetPropertyValue_InContainer(this);
}

bool AActor::GetTearOff() const
{
    static auto bTearOff = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bTearOff")));

    return bTearOff->GetPropertyValue_InContainer(this);
}

bool AActor::IsNetStartupActor() const
{
    static auto bNetStartup = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bNetStartup")));

    return bNetStartup->GetPropertyValue_InContainer(this);
}

ENetRole AActor::GetRemoteRole() const
{
    static auto RemoteRole = GetClass()->FindPropertyByName(FName(TEXT("RemoteRole")));

    return (ENetRole)*RemoteRole->ContainerPtrToValuePtr<uint8>(this);
}

bool AActor::HasAuthority() const
{
    static auto Role = GetClass()->FindPropertyByName(FName(TEXT("Role")));

    return (ENetRole)*Role->ContainerPtrToValuePtr<uint8>(this) == ROLE_Authority;
}

ENetDormancy AActor::GetNetDormancy() const
{
    static auto NetDormancy = GetClass()->FindPropertyByName(FName(TEXT("NetDormancy")));

    return (ENetDormancy)*NetDormancy->ContainerPtrToValuePtr<uint8>(this);
}

float AActor::GetNetUpdateFrequency() const
{
    static auto NetUpdateFrequency = GetClass()->FindPropertyByName(FName(TEXT("NetUpdateFrequency")));

    return *NetUpdateFrequency->ContainerPtrToValuePtr<float>(this);
}

float AActor::GetMinNetUpdateFrequency() const
{
    static auto MinNetUpdateFrequency = GetClass()->FindPropertyByName(FName(TEXT("MinNetUpdateFrequency")));

    return *MinNetUpdateFrequency->ContainerPtrToValuePtr<float>(this);
}

void AActor::SetMinNetUpdateFrequency(float InMinNetUpdateFrequency)
{
    static auto MinNetUpdateFrequency = GetClass()->FindPropertyByName(FName(TEXT("MinNetUpdateFrequency")));

    *MinNetUpdateFrequency->ContainerPtrToValuePtr<float>(this) = InMinNetUpdateFrequency;
}

int32 AActor::GetNetTag() const
{
    static auto NetTag = GetClass()->FindPropertyByName(FName(TEXT("NetTag")));

    return *NetTag->ContainerPtrToValuePtr<int32>(this);
}

void AActor::SetNetTag(int32 InNetTag)
{
    static auto NetTag = GetClass()->FindPropertyByName(FName(TEXT("NetTag")));

    *NetTag->ContainerPtrToValuePtr<int32>(this) = InNetTag;
}

FName AActor::GetNetDriverName() const
{
    static auto NetDriverName = GetClass()->FindPropertyByName(FName(TEXT("NetDriverName")));

    return *NetDriverName->ContainerPtrToValuePtr<FName>(this);
}

USceneComponent* AActor::GetRootComponent() const
{
    static auto RootComponent = GetClass()->FindPropertyByName(FName(TEXT("RootComponent")));

    return *RootComponent->ContainerPtrToValuePtr<USceneComponent*>(this);
}

bool AActor::IsPendingKillPending() const
{
    static auto bActorIsBeingDestroyed = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("Destroying %s, which doesn't have a valid world pointer"));
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));

        for (auto Address = Reference; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xF6 && Instruction.modrm_reg == 0 && Instruction.modrm_mod == 2)
                return std::pair((int32_t)Instruction.disp.disp32, Instruction.imm.imm8);

            Address += Length;
        }

        return std::pair<int32_t, uint8_t>();
    }();

    return (*((uint8_t*)this + bActorIsBeingDestroyed.first) & bActorIsBeingDestroyed.second) || IsPendingKill();
}

bool AActor::IsActorInitialized() const
{
    static auto bActorInitialized = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "));
        auto Function = Scanner::FindFunction(Reference);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < Reference;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xF6 && Instruction.modrm_reg == 0 && Instruction.modrm_mod == 2)
                return std::pair((int32_t)Instruction.disp.disp32, Instruction.imm.imm8);

            Address += Length;
        }

        return std::pair<int32_t, uint8_t>();
    }();

    return *((uint8_t*)this + bActorInitialized.first) & bActorInitialized.second;
}

ULevel* AActor::GetLevel() const
{
    return (ULevel*)GetOuter();
}

const AActor* AActor::GetNetOwner() const
{
    static auto GetNetOwner = [this]
    {
        auto Owner = GetClass()->FindPropertyByName(FName(TEXT("Owner")))->GetOffset_ForInternal();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = 0;

        while (memcmp(VTable[Index], "\x48\x8B\x81", 3) || *(int32_t*)(VTable[Index] + 3) != Owner || VTable[Index][7] != 0xC3)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    return (*(const AActor* (**)(const AActor*))(VTable + GetNetOwner))(this);
}

bool AActor::IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const
{
    static auto IsRelevancyOwnerFor = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = 0;

        while (memcmp(VTable[Index], "\x4C\x3B\xC1\x0F\x94\xC0\xC3", 7))
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AActor*, const AActor*, const AActor*, const AActor*))(VTable + IsRelevancyOwnerFor))(this, ReplicatedActor, ActorOwner, ConnectionActor);
}

void AActor::ForceNetUpdate()
{
    static auto ForceNetUpdate = []
    {
        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Actor:ForceNetUpdate")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            Address += Length;
        }
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(AActor*))(VTable + ForceNetUpdate))(this);
}

bool AActor::Destroy(bool bNetForce, bool bShouldModifyLevel)
{
    static auto Destroy = []
    {
        int32_t K2_DestroyActor = 0;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Actor:K2_DestroyActor")))->GetNativeFunc()); !K2_DestroyActor;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
                K2_DestroyActor = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));

        for (auto Address = VTable[K2_DestroyActor / sizeof(void*)];;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 || Instruction.opcode == 0xE9)
                return (bool (*)(AActor*, bool, bool))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    return Destroy(this, bNetForce, bShouldModifyLevel);
}
