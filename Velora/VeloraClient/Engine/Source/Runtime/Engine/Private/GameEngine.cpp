#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Console.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/OutputDeviceRedirector.h"
#include "CoreGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

UGameInstance* UGameEngine::GetGameInstance() const
{
    static auto GameInstance = GetClass()->FindPropertyByName(FName(TEXT("GameInstance")));

    return *GameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

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
    }

    auto GameInstance = GetGameInstance();
    auto LocalPlayer = GameInstance && GameInstance->GetLocalPlayers().Num() ? GameInstance->GetLocalPlayers()[0] : nullptr;

    if (auto PlayerController = LocalPlayer ? LocalPlayer->GetPlayerController() : nullptr)
        PlayerController->AddCheats(true);

    Original(DeltaSeconds, bIdleMode);
}
