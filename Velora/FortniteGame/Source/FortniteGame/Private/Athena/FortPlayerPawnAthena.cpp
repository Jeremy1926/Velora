#include "pch.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "GameFramework/Controller.h"
#include "FortRegisteredPlayerInfo.h"
#include "Items/FortQuickBars.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

static auto OnPlayerPawnReady = Detour(TEXT("AFortPlayerPawnAthena::OnPlayerPawnReady"), Scanner::CallTo(Scanner::String(TEXT("ExitAircraft: %s (Pawn %s)")).Start().Get()).Start().Get(), &AFortPlayerPawnAthena::OnPlayerPawnReady);

void AFortPlayerPawnAthena::OnPlayerPawnReady()
{
    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetController());

    if (PlayerController)
    {
        if (auto MyPlayerInfo = PlayerController->GetMyPlayerInfo())
            MyPlayerInfo->UpdateAbilitySystemActor(false);
    }

    (this->*::OnPlayerPawnReady)();

    if (PlayerController)
    {
        PlayerController->ActivateSlot(EFortQuickBars::Primary, 0, 0.f, true);

        if (auto QuickBars = PlayerController->GetQuickBars())
        {
            auto& Slots = QuickBars->GetQuickBar(EFortQuickBars::Primary).Slots;

            if (Slots.Num() && Slots[0].Items.Num())
                PlayerController->ClientExecuteInventoryItem(Slots[0].Items[0], 0.0f, false);
        }
    }
}
