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
    static auto GetInterfaceAddress = Scanner::Function<void* (*)(UObjectBaseUtility*, UClass*)>(Scanner::At(Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent")).Find("ret").Last("call 0x*").Call().Find("jmp 0x*").Target()).Call(2).Get(TEXT("UObjectBaseUtility::GetInterfaceAddress")));

    return GetInterfaceAddress(this, InterfaceClass);
}
