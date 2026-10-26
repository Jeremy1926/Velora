#include "pch.h"
#include "Player/FortPlayerController.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Console.h"
#include "Pawns/FortPawn.h"
#include "Abilities/FortMovementSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/Parse.h"
#include "Scanner.h"
#include "Detour.h"

void AFortPlayerController::Cheat(const FString& Msg)
{
    DETOUR(AFortPlayerController, Cheat, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerController:Cheat"))
        .Inline());

    auto Stream = *Msg;

    if (FParse::Command(&Stream, TEXT("FlySpeed")))
    {
        auto FortPawn = (AFortPawn*)GetPawn();
        auto MovementSet = FortPawn ? FortPawn->GetMovementSet() : nullptr;
        auto FlySpeed = (float)_wtof(Stream);

        if (auto FlySpeedAttribute = MovementSet ? MovementSet->GetFlySpeed() : nullptr)
        {
            FlySpeedAttribute->SetBaseValue(FlySpeed);
            FlySpeedAttribute->SetCurrentValue(FlySpeed);
        }
        else if (auto CharacterMovement = FortPawn ? FortPawn->GetCharacterMovement() : nullptr)
            CharacterMovement->SetMaxFlySpeed(FlySpeed);
    }
    else if (FParse::Command(&Stream, TEXT("Speed")))
    {
        auto FortPawn = (AFortPawn*)GetPawn();

        if (auto MovementSet = FortPawn ? FortPawn->GetMovementSet() : nullptr)
        {
            auto SpeedMultiplier = (float)_wtof(Stream);

            MovementSet->GetSpeedMultiplier().SetBaseValue(SpeedMultiplier);
            MovementSet->GetSpeedMultiplier().SetCurrentValue(SpeedMultiplier);

            if (!StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPawn:SetMovementSpeedMultiplier")))
                FortPawn->SetCustomTimeDilation(FMath::Max(SpeedMultiplier, 0.0f));
        }
    }

    if (!FParse::Command(&Stream, TEXT("Help")))
    {
        Original(Msg);
        return;
    }

    auto GameViewport = GEngine->GetGameViewport();
    auto Console = GameViewport ? GameViewport->GetViewportConsole() : nullptr;

    for (auto& Command : GCheatCommands)
    {
        FString Line(Command[0]);
        Line += TEXT(" - ");
        Line += Command[1];

        if (Console)
            Console->Serialize(*Line, ELogVerbosity::Log, FName(TEXT("LogConsoleResponse")));
    }
}
