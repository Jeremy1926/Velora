#include "pch.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Scanner.h"

void IGameplayTagAssetInterface::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GetOwnedGameplayTags = Anchor::VirtualFunction<void (*)(const IGameplayTagAssetInterface*, FGameplayTagContainer&)>(TEXT("IGameplayTagAssetInterface::GetOwnedGameplayTags"))
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:GetOwnedGameplayTags"))
        .Find("call [*+0x*]")
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
        .Find("setnz *; add r*, r*; mov [*+0x20], r*")
        .Find("call [rax+0x*]")
        .Get();

    return HasMatchingGameplayTag(this, TagToCheck);
}
