#include "pch.h"
#include "Player/FortTeamPrivateInfo.h"
#include "Scanner.h"

TMulticastDelegate<void, AFortTeamPrivateInfo*, AFortPlayerState*>& AFortTeamPrivateInfo::GetNotifyPlayerAdded()
{
    static auto NotifyPlayerAdded = Anchor::Data<TMulticastDelegate<void, AFortTeamPrivateInfo*, AFortPlayerState*>*>(TEXT("AFortTeamPrivateInfo::NotifyPlayerAdded"))
        .String(TEXT("Adding %s to team %s"))
        .Find("test byte ptr [*+0x*], 0x8; jnz *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyPlayerAdded;
}

TMulticastDelegate<void, AFortTeamPrivateInfo*, AFortPlayerState*>& AFortTeamPrivateInfo::GetNotifyPlayerRemoved()
{
    static auto NotifyPlayerRemoved = Anchor::Data<TMulticastDelegate<void, AFortTeamPrivateInfo*, AFortPlayerState*>*>(TEXT("AFortTeamPrivateInfo::NotifyPlayerRemoved"))
        .String(TEXT("Removing %s from team %s"))
        .Find("test byte ptr [*+0x*], 0x8; jnz *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyPlayerRemoved;
}
