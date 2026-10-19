#include "pch.h"
#include "FortRegisteredPlayerInfo.h"
#include "Scanner.h"

IAbilitySystemInterface* UFortRegisteredPlayerInfo::UpdateAbilitySystemActor(bool bForceUpdate)
{
    static auto UpdateAbilitySystemActor = Scanner::Function<IAbilitySystemInterface* (*)(UFortRegisteredPlayerInfo*, bool)>(Scanner::String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]")).Start().Get(TEXT("UFortRegisteredPlayerInfo::UpdateAbilitySystemActor")));

    return UpdateAbilitySystemActor(this, bForceUpdate);
}

IAbilitySystemInterface* UFortRegisteredPlayerInfo::PlayerPawnReadyUpdateAbilitySystemActor(bool bForceUpdate)
{
    UpdateAbilitySystemActor(false);

    return UpdateAbilitySystemActor(bForceUpdate);
}
