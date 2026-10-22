#include "pch.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/Class.h"
#include "Scanner.h"

bool UObjectBaseUtility::IsA(const UClass* SomeBase) const
{
    return GetClass()->IsChildOf(SomeBase);
}

void* UObjectBaseUtility::GetInterfaceAddress(UClass* InterfaceClass)
{
    static auto GetInterfaceAddress = Anchor::Function<void* (*)(UObjectBaseUtility*, UClass*)>(TEXT("UObjectBaseUtility::GetInterfaceAddress"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Find("jmp 0x*")
        .Target()
        .Call(2)
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Call(2)
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("mov rdx, rax; mov rcx, *; call 0x*; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetInterfaceAddress(this, InterfaceClass);
}
