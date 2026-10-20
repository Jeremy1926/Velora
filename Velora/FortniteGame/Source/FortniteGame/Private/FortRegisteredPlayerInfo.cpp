#include "pch.h"
#include "FortRegisteredPlayerInfo.h"
#include "Scanner.h"

IAbilitySystemInterface* UFortRegisteredPlayerInfo::UpdateAbilitySystemActor(bool bForceUpdate)
{
    static auto UpdateAbilitySystemActor = Anchor::Function<IAbilitySystemInterface* (*)(UFortRegisteredPlayerInfo*, bool)>(TEXT("UFortRegisteredPlayerInfo::UpdateAbilitySystemActor"))
        .String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .Start()
        .Get();

    return UpdateAbilitySystemActor(this, bForceUpdate);
}

IAbilitySystemInterface* UFortRegisteredPlayerInfo::PlayerPawnReadyUpdateAbilitySystemActor(bool bForceUpdate)
{
    UpdateAbilitySystemActor(false);

    return UpdateAbilitySystemActor(bForceUpdate);
}
