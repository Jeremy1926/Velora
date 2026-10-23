#include "pch.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/Class.h"
#include "Scanner.h"

bool UObjectBaseUtility::IsA(const UClass* SomeBase) const
{
    return GetClass()->IsChildOf(SomeBase);
}

UObject* UObjectBaseUtility::GetOutermost() const
{
    static auto GetOutermost = Anchor::Function<UObject* (*)(const UObjectBaseUtility*)>(TEXT("UObjectBaseUtility::GetOutermost"))
        .Pattern("40 53 48 83 EC 20 48 8D 59 20 48 83 3B 00 74 ? 33 C0 F7 41 08 00 00 00 10 75 ? 48 8B C8 48 85 C9 75 ? 48 8B 0B EB ? 48 8B C1 48 83 C4 20 5B C3")
        .Get();

    return GetOutermost(this);
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
