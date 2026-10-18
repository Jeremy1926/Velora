#include "pch.h"
#include "GameplayAbilitySpec.h"
#include "Scanner.h"

void FGameplayAbilitySpecHandle::GenerateNewHandle()
{
    static auto& GHandle = *(int32*)Scanner::String(TEXT("FGameplayAbilitySpec::FGameplayAbilitySpec for %s from %s")).Start().Find("inc [0x*]").Target(TEXT("FGameplayAbilitySpecHandle::GenerateNewHandle::GHandle"));

    Handle = GHandle++;
}
