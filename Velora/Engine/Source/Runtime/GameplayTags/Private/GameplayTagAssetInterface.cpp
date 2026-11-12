#include "pch.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Scanner.h"

void IGameplayTagAssetInterface::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GetOwnedGameplayTags = Anchor::VirtualFunction<void (*)(const IGameplayTagAssetInterface*, FGameplayTagContainer&)>(TEXT("IGameplayTagAssetInterface::GetOwnedGameplayTags"))
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:GetOwnedGameplayTags"))
        .Find("setnz *; add r*, r*")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:GetOwnedGameplayTags"))
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayTags.BlueprintGameplayTagLibrary:GetOwnedGameplayTags"))
        .Find("lea rdx, [rsp+0x*]; lea rcx, [rsp+0x*]; call 0x*; mov rdx, rax")
        .Find("call 0x*")
        .Target()
        .Find("mov rdx, rbx; mov rax, [rcx]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayTags.BlueprintGameplayTagLibrary:GetOwnedGameplayTags"))
        .Find("mov [rsp+0x38], rax")
        .Find("call 0x*")
        .Target()
        .Find("cmp [rdx+0x8], rax; jz 0x*; mov rcx, [rdx+0x8]")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayTags.BlueprintGameplayTagLibrary:GetOwnedGameplayTags"))
        .Find("sbb rcx, rcx; and rcx, rax; mov rax, [rcx]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    GetOwnedGameplayTags(this, TagContainer);
}

bool IGameplayTagAssetInterface::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
    static auto HasMatchingGameplayTag = Anchor::VirtualFunction<bool (*)(const IGameplayTagAssetInterface*, FGameplayTag)>(TEXT("IGameplayTagAssetInterface::HasMatchingGameplayTag"))
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:HasMatchingGameplayTag"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:HasMatchingGameplayTag"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:HasMatchingGameplayTag"))
        .Find("setnz *; add r*, r*")
        .Find("call [rax+0x*]")
        .Get();

    return HasMatchingGameplayTag(this, TagToCheck);
}
