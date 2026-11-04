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
    static auto GetOutermost = Anchor::Optional<UObject* (*)(const UObjectBaseUtility*)>(TEXT("UObjectBaseUtility::GetOutermost"))
        .Pattern("40 53 48 83 EC 20 48 8D 59 20 48 83 3B 00 74 ? 33 C0 F7 41 08 00 00 00 10 75 ? 48 8B C8 48 85 C9 75 ? 48 8B 0B EB ? 48 8B C1 48 83 C4 20 5B C3")
        .Or()
        .Pattern("40 53 48 83 EC 20 48 8D 59 20 48 83 3B 00 74 ? 33 C0 F7 41 08 00 00 00 10 74 ? E8 ? ? ? ? 48 8B C8 48 85 C9 75 ? 48 8B 0B EB ? 48 8B C1 48 83 C4 20 5B C3")
        .Or()
        .Pattern("40 53 48 83 EC 20 48 8D 59 20 48 83 3B 00 74 ? 8B 41 08 33 D2 0F BA E0 1C 0F 82 ? ? ? ? 48 8B CA 48 85 C9 75 ? 48 8B 0B EB ? 48 8B C1 48 83 C4 20 5B C3")
        .Or()
        .Pattern("40 53 48 83 EC 20 48 8D 59 20 48 83 3B 00 74 ? 8B 41 08 33 D2 0F BA E0 1C 73 ? E8 ? ? ? ? 48 8B D0 48 8B CA 48 85 C9 75 ? 48 8B 0B EB ? 48 8B C1 48 83 C4 20 5B C3")
        .Get();

    if (GetOutermost)
        return GetOutermost(this);

    static auto GetObjectExternalPackageThreadSafe = Anchor::Function<UObject* (*)(const UObjectBaseUtility*)>(TEXT("GetObjectExternalPackageThreadSafe"))
        .Pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 E8 ? ? ? ? 80 3D ? ? ? ? 00 48 8B F8 74 ? E8 ? ? ? ? 84 C0 74 ? 33 DB EB ? 48 8B CF 48 8B DF FF 15")
        .Or()
        .Pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 48 8B F1 E8 ? ? ? ? 80 3D ? ? ? ? 00 48 8B F8 74 ? 80 3D ? ? ? ? 00 74 ? 65 48 8B 14 25 58 00 00 00")
        .Get();

    for (const UObjectBaseUtility* Top = this;; Top = Top->GetOuter())
    {
        if (!Top->GetOuter())
            return (UObject*)Top;

        if (Top->GetFlags() & RF_HasExternalPackage)
        {
            if (auto Package = GetObjectExternalPackageThreadSafe(Top))
                return Package;
        }
    }
}

void* UObjectBaseUtility::GetInterfaceAddress(UClass* InterfaceClass)
{
    static auto GetInterfaceAddress = Anchor::Function<void* (*)(UObjectBaseUtility*, UClass*)>(TEXT("UObjectBaseUtility::GetInterfaceAddress"))
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveFortItemFromPlayer"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("call 0x*")
        .Target()
        .Find("mov rdx, rax; call 0x*; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("STAT_McpItem_PostAdd"))
        .Last("mov rdx, rax; mov rcx, *; call 0x*; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("mov rdx, rax; mov rcx, *; call 0x*; mov rcx, rax; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("mov rdx, [0x*]; mov rcx, *; call 0x*; mov rcx, rax; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("mov rdx, [0x*]; mov rcx, *; call 0x*; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Find("jmp 0x*")
        .Target()
        .Call(2)
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("mov rdx, rax; mov rcx, *; call 0x*; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:GetAbilitySystemComponent"))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Call(2)
        .Get();

    return GetInterfaceAddress(this, InterfaceClass);
}
