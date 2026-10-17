#include "pch.h"
#include "FortRegisteredPlayerInfo.h"
#include "Scanner.h"

IAbilitySystemInterface* UFortRegisteredPlayerInfo::UpdateAbilitySystemActor(bool bForceUpdate)
{
    static auto UpdateAbilitySystemActor = (IAbilitySystemInterface* (*)(UFortRegisteredPlayerInfo*, bool))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]")))->BeginAddress);

    return UpdateAbilitySystemActor(this, bForceUpdate);
}
