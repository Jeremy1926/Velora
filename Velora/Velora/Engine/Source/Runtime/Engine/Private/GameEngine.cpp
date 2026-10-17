#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto Tick = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)")))->BeginAddress, &UGameEngine::Tick);

UGameInstance* UGameEngine::GetGameInstance() const
{
    static auto GameInstance = GetClass()->FindPropertyByName(FName(TEXT("GameInstance")));

    return *GameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

void UGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
    auto GameInstance = GetGameInstance();

    if (!GIsServer)
    {
        GIsClient = false;
        GIsServer = true;

        UGameplayStatics::OpenLevel(GameInstance, FName(TEXT("/Game/Athena/Maps/Athena_Terrain")));
    }
    else if (auto LocalPlayer = GameInstance->GetFirstGamePlayer())
    {
        auto World = GetGameViewport()->GetWorld();

        if (World && World->GetNetDriver())
            GameInstance->RemoveLocalPlayer(LocalPlayer);
    }

    (this->*::Tick)(DeltaSeconds, bIdleMode);
}
