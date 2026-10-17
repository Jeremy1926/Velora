#include "pch.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

struct AbilitySystemComponent_eventClientActivateAbilityFailed_Parms
{
    FGameplayAbilitySpecHandle AbilityToActivate;
    int16 PredictionKey;
};

static const uint8_t* FindGiveAbility()
{
    auto Address = Scanner::FindStringReference(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"));

    for (auto Calls = 0;;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8 && ++Calls == 3)
            return Address + Length + (int32_t)Instruction.imm.imm32;

        Address += Length;
    }
}

FGameplayTagContainer& UAbilitySystemComponent::InternalTryActivateAbilityFailureTags = *[]
{
    for (auto Address = (const uint8_t*)Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("InternalTryActivateAbility called with invalid Ability")))->BeginAddress;;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0x8D && Instruction.rex_w && Instruction.modrm == 0x0D)
            return (FGameplayTagContainer*)(Address + Length + (int32_t)Instruction.disp.disp32);

        Address += Length;
    }
}();

FGameplayAbilitySpecHandle UAbilitySystemComponent::GiveAbility(const FGameplayAbilitySpec& Spec)
{
    static auto GiveAbility = (FGameplayAbilitySpecHandle* (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, const FGameplayAbilitySpec&))FindGiveAbility();

    static auto InternalServerTryActiveAbility = [this]
    {
        auto Thunk = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerTryActivateAbility")))->GetNativeFunc());
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Thunk));
        int32_t ServerTryActivateAbility_Implementation = 0;

        for (auto Address = Thunk; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                ServerTryActivateAbility_Implementation = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto VTable = *(uint8_t**)this;

        for (auto Address = *(const uint8_t**)(VTable + ServerTryActivateAbility_Implementation);;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                return DetourVTable((void**)(VTable + (int32_t)Instruction.disp.disp32), &UAbilitySystemComponent::InternalServerTryActiveAbility);

            Address += Length;
        }
    }();

    FGameplayAbilitySpecHandle Handle;

    GiveAbility(this, &Handle, Spec);

    return Handle;
}

FGameplayAbilitySpec* UAbilitySystemComponent::FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle Handle)
{
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities().Items)
    {
        if (Spec.Handle == Handle)
        {
            return &Spec;
        }
    }

    return nullptr;
}

void UAbilitySystemComponent::MarkAbilitySpecDirty(FGameplayAbilitySpec& Spec)
{
    static auto MarkAbilitySpecDirty = []
    {
        auto GiveAbility = FindGiveAbility();
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(GiveAbility));
        const uint8_t* Call = nullptr;

        for (auto Address = GiveAbility; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                Call = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (void (*)(UAbilitySystemComponent*, FGameplayAbilitySpec&))Call;
    }();

    MarkAbilitySpecDirty(this, Spec);
}

bool UAbilitySystemComponent::InternalTryActivateAbility(FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
    static auto InternalTryActivateAbility = (bool (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, FPredictionKey*, UGameplayAbility**, FOnGameplayAbilityEnded::FDelegate*, const FGameplayEventData*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("InternalTryActivateAbility called with invalid Ability")))->BeginAddress);

    return InternalTryActivateAbility(this, Handle, &InPredictionKey, OutInstancedAbility, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

void UAbilitySystemComponent::ClientActivateAbilityFailed(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey)
{
    static FName NAME_UAbilitySystemComponent_ClientActivateAbilityFailed = FName(TEXT("ClientActivateAbilityFailed"));

    AbilitySystemComponent_eventClientActivateAbilityFailed_Parms Parms;
    Parms.AbilityToActivate = AbilityToActivate;
    Parms.PredictionKey = PredictionKey;
    ProcessEvent(FindFunctionChecked(NAME_UAbilitySystemComponent_ClientActivateAbilityFailed), &Parms);
}

void UAbilitySystemComponent::InternalServerTryActiveAbility(FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
    if (!Spec)
    {
        ABILITY_LOG(Display, TEXT("InternalServerTryActiveAbility. Rejecting ClientActivation of ability with invalid SpecHandle!"));
        ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        return;
    }

    ConsumeAllReplicatedData(Handle, PredictionKey);

    FScopedPredictionWindow ScopedPredictionWindow(this, PredictionKey);

    UGameplayAbility* InstancedAbility = nullptr;
    Spec->InputPressed = true;

    if (!InternalTryActivateAbility(Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
        ABILITY_LOG(Display, TEXT("InternalServerTryActiveAbility. Rejecting ClientActivation of %s. InternalTryActivateAbility failed: %s"), *GetNameSafe(Spec->Ability), *InternalTryActivateAbilityFailureTags.ToStringSimple());
        ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
    }
    MarkAbilitySpecDirty(*Spec);
}

void UAbilitySystemComponent::ConsumeAllReplicatedData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey)
{
    FAbilityReplicatedDataCache* CachedData = GetAbilityTargetDataMap().Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, AbilityOriginalPredictionKey));
    if (CachedData)
    {
        CachedData->Reset();
    }
}

FGameplayAbilitySpecContainer& UAbilitySystemComponent::GetActivatableAbilities()
{
    static auto ActivatableAbilities = GetClass()->FindPropertyByName(FName(TEXT("ActivatableAbilities")));

    return *ActivatableAbilities->ContainerPtrToValuePtr<FGameplayAbilitySpecContainer>(this);
}

TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>& UAbilitySystemComponent::GetAbilityTargetDataMap()
{
    static auto AbilityTargetDataMap = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Ability %s is overriding pending replicated target data.")));
        auto End = Scanner::FindFunctionEnd(Function);
        auto bScopedPredictionWindow = false;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bScopedPredictionWindow && Instruction.opcode == 0x8D && Instruction.modrm_mod == 2)
                return (int32_t)Instruction.disp.disp32;

            bScopedPredictionWindow |= Instruction.opcode == 0xE8;
            Address += Length;
        }

        return 0;
    }();

    return *(TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>*)((uint8_t*)this + AbilityTargetDataMap);
}
