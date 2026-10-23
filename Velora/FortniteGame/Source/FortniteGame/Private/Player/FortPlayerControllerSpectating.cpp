#include "pch.h"
#include "Player/FortPlayerControllerSpectating.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Pawns/FortPlayerPawn.h"
#include "Player/FortPlayerState.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

AFortPlayerState* AFortPlayerControllerSpectating::GetFollowedPlayer() const
{
    static auto GetFollowedPlayer = Anchor::VirtualFunction<AFortPlayerState* (*)(const AFortPlayerControllerSpectating*)>(TEXT("AFortPlayerControllerSpectating::GetFollowedPlayer"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerControllerSpectating:GetFollowedPlayer"))
        .Find("call [rax+0x*]")
        .Get();

    return GetFollowedPlayer(this);
}

APawn* AFortPlayerControllerSpectating::GetFollowedPlayerPawn(bool bAlwaysReturnPlayer) const
{
    auto FollowedPlayer = GetFollowedPlayer();
    auto CurrentPawn = FollowedPlayer ? Cast<AFortPlayerPawn>(FollowedPlayer->GetCurrentPawn()) : nullptr;

    if (!bAlwaysReturnPlayer && CurrentPawn && CurrentPawn->GetVehicle())
        return (APawn*)CurrentPawn->GetVehicle();

    return CurrentPawn;
}

AFortBroadcastRemoteClientInfo* AFortPlayerControllerSpectating::GetFollowedPlayerRemoteClientInfo() const
{
    static auto FollowedPlayerRemoteClientInfo = GetClass()->FindPropertyByName(FName(TEXT("FollowedPlayerRemoteClientInfo")));

    return *FollowedPlayerRemoteClientInfo->ContainerPtrToValuePtr<AFortBroadcastRemoteClientInfo*>(this);
}
