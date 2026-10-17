#include "pch.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "GameFramework/Controller.h"
#include "FortRegisteredPlayerInfo.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

static auto OnPlayerPawnReady = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindCallReference(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("ExitAircraft: %s (Pawn %s)")))->BeginAddress))->BeginAddress, &AFortPlayerPawnAthena::OnPlayerPawnReady);

void AFortPlayerPawnAthena::OnPlayerPawnReady()
{
    if (auto PlayerController = Cast<AFortPlayerControllerAthena>(GetController()))
    {
        if (auto MyPlayerInfo = PlayerController->GetMyPlayerInfo())
            MyPlayerInfo->UpdateAbilitySystemActor(false);
    }

    (this->*::OnPlayerPawnReady)();
}
