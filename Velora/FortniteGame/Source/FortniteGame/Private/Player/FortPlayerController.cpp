#include "pch.h"
#include "Player/FortPlayerController.h"
#include "Items/FortQuickBars.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

static auto InitializeMcpProfile = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Initialized MCP profile for %s")))->BeginAddress, &AFortPlayerController::InitializeMcpProfile);

bool AFortPlayerController::InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized)
{
    auto bInitialized = (this->*::InitializeMcpProfile)(OnMcpProfilesInitialized);

    if (HasAuthority() && !GetQuickBars())
    {
        FActorSpawnParameters SpawnInfo;
        SpawnInfo.Owner = this;

        GetQuickBars() = (AFortQuickBars*)GetWorld()->SpawnActor(AFortQuickBars::StaticClass(), nullptr, SpawnInfo);
    }

    return bInitialized;
}

AFortQuickBars*& AFortPlayerController::GetQuickBars()
{
    static auto QuickBars = GetClass()->FindPropertyByName(FName(TEXT("QuickBars")));

    return *QuickBars->ContainerPtrToValuePtr<AFortQuickBars*>(this);
}
