#include "pch.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "GameFramework/Controller.h"
#include "FortRegisteredPlayerInfo.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

static auto OnPlayerPawnReady = Detour(TEXT("AFortPlayerPawnAthena::OnPlayerPawnReady"), Scanner::CallTo(Scanner::String(TEXT("ExitAircraft: %s (Pawn %s)")).Start().Get()).Start().Get(), &AFortPlayerPawnAthena::OnPlayerPawnReady);

void AFortPlayerPawnAthena::OnPlayerPawnReady()
{
    if (auto PlayerController = Cast<AFortPlayerControllerAthena>(GetController()))
    {
        if (auto MyPlayerInfo = PlayerController->GetMyPlayerInfo())
            MyPlayerInfo->UpdateAbilitySystemActor(false);
    }

    (this->*::OnPlayerPawnReady)();
}
