#include "pch.h"
#include "FortAthenaLivingWorldPlayerTracker.h"
#include "AI/FortAthenaAIBotController.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Engine/World.h"
#include "Pawns/FortPlayerPawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

template<>
void TBVTree<FSimpleBVTreeElement>::RecreateTree(const TArray<FSimpleBVTreeElement>& InElements)
{
    static auto CreateCommonInternal = Anchor::Function<void (*)(TBVTree<FSimpleBVTreeElement>*)>(TEXT("TBVTree<FSimpleBVTreeElement>::CreateCommonInternal"))
        .String(TEXT("STAT_FortAI_UpdateSpatialLookups"))
        .Start()
        .Find("lea rdx, [*+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, r1*; call 0x*; cmp qword ptr [r1*], 0x0")
        .Find("call 0x*")
        .Target()
        .Get();

    Nodes.Reset();
    NodeBoundingBoxes.Reset();
    Elements = InElements;

    CreateCommonInternal(this);
}

UScriptStruct* FFortAthenaLivingWorldPlayerTracker::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPlayerTracker"));

    return Struct;
}

TSet<const AFortAthenaAIBotController*>& FFortAthenaLivingWorldPlayerTracker::GetTrackedBotControllers()
{
    static auto TrackedBotControllers = StaticStruct()->FindPropertyByName(FName(TEXT("TrackedBotControllers")));

    return *TrackedBotControllers->ContainerPtrToValuePtr<TSet<const AFortAthenaAIBotController*>>(this);
}

TArray<const AFortPlayerPawn*>& FFortAthenaLivingWorldPlayerTracker::GetTrackedPlayerPawns()
{
    static auto TrackedPlayerPawns = StaticStruct()->FindPropertyByName(FName(TEXT("TrackedPlayerPawns")));

    return *TrackedPlayerPawns->ContainerPtrToValuePtr<TArray<const AFortPlayerPawn*>>(this);
}

TArray<const AController*>& FFortAthenaLivingWorldPlayerTracker::GetTrackedPlayerControllers()
{
    static auto TrackedPlayerControllers = StaticStruct()->FindPropertyByName(FName(TEXT("TrackedPlayerControllers")));

    return *TrackedPlayerControllers->ContainerPtrToValuePtr<TArray<const AController*>>(this);
}

TBVTree<FSimpleBVTreeElement>& FFortAthenaLivingWorldPlayerTracker::GetTrackedPlayerBVTree()
{
    return *(TBVTree<FSimpleBVTreeElement>*)(&GetTrackedPlayerControllers() + 1);
}

int32& FFortAthenaLivingWorldPlayerTracker::GetMinNumberOfPlayers()
{
    return *(int32*)(&GetTrackedPlayerBVTree() + 1);
}

float& FFortAthenaLivingWorldPlayerTracker::GetTimeSinceLastRefresh()
{
    return *(float*)(&GetMinNumberOfPlayers() + 1);
}

bool& FFortAthenaLivingWorldPlayerTracker::GetbHasSelectedPlayerBots()
{
    return *(bool*)(&GetTimeSinceLastRefresh() + 1);
}

void FFortAthenaLivingWorldPlayerTracker::UpdateTimeSinceLastRefresh(float DeltaTime)
{
    GetTimeSinceLastRefresh() += DeltaTime;
}

void FFortAthenaLivingWorldPlayerTracker::RefreshTree(UWorld* World, const AFortGameStateAthena* GameState)
{
    static auto BusLocked = (EAthenaGamePhaseStep)StaticEnum<EAthenaGamePhaseStep>()->GetValueByName(FName(TEXT("EAthenaGamePhaseStep::BusLocked")));

    GetTimeSinceLastRefresh() = 0.0f;

    auto& TrackedBotControllers = GetTrackedBotControllers();
    auto GameMode = Cast<AFortGameModeAthena>(World->GetAuthGameMode());

    if (!GetbHasSelectedPlayerBots() && GameState->GetGamePhaseStep() >= BusLocked && GameMode)
    {
        const int32 NumAlivePlayers = GameMode->GetAlivePlayers().Num();

        if (NumAlivePlayers < GetMinNumberOfPlayers())
        {
            const int32 NumBotsToTrack = GetMinNumberOfPlayers() - NumAlivePlayers;

            for (auto BotController : GameMode->GetAliveBots())
            {
                if (!BotController)
                    continue;

                TrackedBotControllers.Add(BotController);

                if (TrackedBotControllers.Num() >= NumBotsToTrack)
                    break;
            }

            GetbHasSelectedPlayerBots() = TrackedBotControllers.Num() >= NumBotsToTrack;
        }
        else
        {
            GetbHasSelectedPlayerBots() = true;
        }
    }

    if (!GameMode)
        return;

    auto& TrackedPlayerControllers = GetTrackedPlayerControllers();
    auto& TrackedPlayerPawns = GetTrackedPlayerPawns();
    const int32 NumToTrack = GameMode->GetAlivePlayers().Num() + TrackedBotControllers.Num();
    TArray<FSimpleBVTreeElement> TreeElements;

    TreeElements.Reserve(NumToTrack);
    TrackedPlayerControllers.Empty(NumToTrack);
    TrackedPlayerPawns.Empty(NumToTrack);

    for (auto PlayerController : GameMode->GetAlivePlayers())
    {
        auto PlayerPawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;

        if (!PlayerPawn)
            continue;

        TrackedPlayerControllers.Add(PlayerController);
        TrackedPlayerPawns.Add(PlayerPawn);
        TreeElements.Add({ TrackedPlayerControllers.Num() - 1, PlayerPawn->GetActorLocation() });
    }

    for (auto BotController : GameMode->GetAliveBots())
    {
        if (!BotController || !TrackedBotControllers.Num() || !TrackedBotControllers.Contains(BotController))
            continue;

        auto PlayerBotPawn = BotController->GetPlayerBotPawn();

        if (!PlayerBotPawn)
            continue;

        TrackedPlayerControllers.Add(BotController);
        TrackedPlayerPawns.Add(PlayerBotPawn);
        TreeElements.Add({ TrackedPlayerControllers.Num() - 1, PlayerBotPawn->GetActorLocation() });
    }

    GetTrackedPlayerBVTree().RecreateTree(TreeElements);
}
