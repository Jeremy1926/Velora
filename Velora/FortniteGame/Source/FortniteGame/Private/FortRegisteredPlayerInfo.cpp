#include "pch.h"
#include "FortRegisteredPlayerInfo.h"
#include "Scanner.h"

IAbilitySystemInterface* UFortRegisteredPlayerInfo::UpdateAbilitySystemActor(bool bForceUpdate)
{
    static auto UpdateAbilitySystemActor = (IAbilitySystemInterface* (*)(UFortRegisteredPlayerInfo*, bool))Scanner::String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]")).Start().Get();

    return UpdateAbilitySystemActor(this, bForceUpdate);
}
