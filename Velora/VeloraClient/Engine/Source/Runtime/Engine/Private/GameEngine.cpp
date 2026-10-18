#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Console.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/OutputDeviceRedirector.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto Tick = Detour(TEXT("UGameEngine::Tick"), Scanner::String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)")).Start().Get(), &UGameEngine::Tick);

void UGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
    auto GameViewport = GetGameViewport();

    if (GameViewport && !GameViewport->GetViewportConsole())
    {
        GameViewport->SetViewportConsole(NewObject<UConsole>(GameViewport, GetConsoleClass()));
        GLog->AddOutputDevice(GameViewport->GetViewportConsole());
    }

    (this->*::Tick)(DeltaSeconds, bIdleMode);
}
