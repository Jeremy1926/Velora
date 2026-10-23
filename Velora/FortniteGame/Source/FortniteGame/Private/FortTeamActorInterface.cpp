#include "pch.h"
#include "FortTeamActorInterface.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* IFortTeamActorInterface::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortTeamActorInterface"));

    return Class;
}

uint8 IFortTeamActorInterface::GetTeam()
{
    static auto GetTeam = Anchor::VirtualFunction<uint8 (*)(IFortTeamActorInterface*)>(TEXT("IFortTeamActorInterface::GetTeam"))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:GetTeam"))
        .Find("call [rax+0x*]")
        .Get();

    return GetTeam(this);
}
