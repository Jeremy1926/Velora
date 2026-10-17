#include "pch.h"
#include "Engine/GameEngine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto Tick = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindReference(Scanner::FindString(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))))->BeginAddress, &UGameEngine::Tick);

UGameInstance* UGameEngine::GetGameInstance() const
{
    static auto GameInstance = GetClass()->FindPropertyByName(FName(TEXT("GameInstance")));

    return *GameInstance->ContainerPtrToValuePtr<UGameInstance*>(this);
}

void UGameEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
    if (!GIsServer)
    {
        GIsClient = false;
        GIsServer = true;

        auto GameInstance = GetGameInstance();

        if (auto LocalPlayer = GameInstance->GetFirstGamePlayer())
            GameInstance->RemoveLocalPlayer(LocalPlayer);

        UGameplayStatics::OpenLevel(GameInstance, FName(TEXT("/Game/Athena/Maps/Athena_Terrain")));
    }

    (this->*::Tick)(DeltaSeconds, bIdleMode);
}
