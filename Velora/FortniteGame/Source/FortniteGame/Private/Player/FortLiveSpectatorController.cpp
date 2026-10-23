#include "pch.h"
#include "Player/FortLiveSpectatorController.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortLiveSpectatorController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortLiveSpectatorController"));

    return Class;
}

AFortPlayerState* AFortLiveSpectatorController::GetFollowedPlayerState() const
{
    static auto FollowedPlayerState = GetClass()->FindPropertyByName(FName(TEXT("FollowedPlayerState")));

    return *FollowedPlayerState->ContainerPtrToValuePtr<AFortPlayerState*>(this);
}

void AFortLiveSpectatorController::GetRemoteClientInfos(TArray<AFortBroadcastRemoteClientInfo*>& OutRemoteClientInfos) const
{
    static auto GetRemoteClientInfos = Anchor::VirtualFunction<void (*)(const AFortLiveSpectatorController*, TArray<AFortBroadcastRemoteClientInfo*>&)>(TEXT("AFortLiveSpectatorController::GetRemoteClientInfos"))
        .Exec(TEXT("/Script/FortniteGame.FortLiveSpectatorController:ServerSetFollowedPlayer"))
        .Find("call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get(sizeof(void*));

    GetRemoteClientInfos(this, OutRemoteClientInfos);
}
