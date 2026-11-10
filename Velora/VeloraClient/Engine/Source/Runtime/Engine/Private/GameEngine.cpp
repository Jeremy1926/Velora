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
#include "HAL/IConsoleManager.h"
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

    static auto bAllowEncryption = []
    {
        auto AllowEncryption = IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowEncryption"));

        if (AllowEncryption)
            AllowEncryption->Set(TEXT("1"), ECVF_SetByCode);

        return AllowEncryption != nullptr;
    }();

    static auto bUseIrisReplication = []
    {
        auto UseIrisReplication = IConsoleManager::Get().FindConsoleVariable(TEXT("net.Iris.UseIrisReplication"));

        if (UseIrisReplication)
            UseIrisReplication->Set(TEXT("0"), ECVF_SetByCode);

        return UseIrisReplication != nullptr;
    }();

    Original(DeltaSeconds, bIdleMode);
}
