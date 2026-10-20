#include "pch.h"
#include "GameplayTagAssetInterface.h"
#include "Scanner.h"

void IGameplayTagAssetInterface::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GetOwnedGameplayTags = Scanner::Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:GetOwnedGameplayTags")).Find("call [*+0x*]").Slot(TEXT("IGameplayTagAssetInterface::GetOwnedGameplayTags"));

    Scanner::Virtual<void (*)(const IGameplayTagAssetInterface*, FGameplayTagContainer&)>(this, GetOwnedGameplayTags)(this, TagContainer);
}
