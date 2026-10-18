#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Engine/Console.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/EngineVersion.h"
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

    static const FString Build = []
    {
        auto& Version = FEngineVersion::Current();
        auto Branch = Version.GetBranch();
        auto Release = wcsstr(*Branch, TEXT("Release-"));

        TCHAR Label[64];

        if (Release && iswdigit(Release[8]))
            swprintf_s(Label, TEXT("FN %s, UE %hu.%hu"), Release + 8, Version.GetMajor(), Version.GetMinor());
        else
            swprintf_s(Label, TEXT("CL %u, UE %hu.%hu"), Version.GetChangelist(), Version.GetMajor(), Version.GetMinor());

        return FString(Label);
    }();

    static bool bTitleSet = false;
    static UWorld* LastWorld = nullptr;
    auto World = GameViewport ? GameViewport->GetWorld() : nullptr;

    if (!bTitleSet || World != LastWorld)
    {
        const TCHAR* State = TEXT("Loading");

        if (World)
            State = wcsstr(*World->GetName(), TEXT("Frontend")) ? TEXT("Lobby") : TEXT("In Match");

        TCHAR Title[128];
        swprintf_s(Title, TEXT("Velora Client (%s): %s"), *Build, State);
        SetConsoleTitleW(Title);

        bTitleSet = true;
        LastWorld = World;
    }

    (this->*::Tick)(DeltaSeconds, bIdleMode);
}
