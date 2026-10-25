#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/Console.h"
#include "GameFramework/InputSettings.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/OutputDeviceRedirector.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"

void UGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
    DETOUR(UGameEngine, Tick, Anchor::Hook()
        .String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))
        .Start()
        .Inline());

    auto GameViewport = GetGameViewport();

    if (GameViewport && !GameViewport->GetViewportConsole())
    {
        GameViewport->SetViewportConsole(NewObject<UConsole>(GameViewport, GetConsoleClass()));
        GLog->AddOutputDevice(GameViewport->GetViewportConsole());

        auto& ConsoleKeys = UInputSettings::GetInputSettings()->GetConsoleKeys();
        FKey F8(FName(TEXT("F8")));

        if (!ConsoleKeys.Contains(F8))
            new(ConsoleKeys.GetData() + ConsoleKeys.AddUninitialized()) FKey(F8.KeyName);
    }

    Original(DeltaSeconds, bIdleMode);
}
