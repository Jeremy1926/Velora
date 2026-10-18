#include "pch.h"
#include "Athena/FortGameStateAthena.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortGameStateAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameStateAthena"));

    return Class;
}

EAthenaGamePhase AFortGameStateAthena::GetGamePhase() const
{
    static auto GamePhase = GetClass()->FindPropertyByName(FName(TEXT("GamePhase")));

    return *GamePhase->ContainerPtrToValuePtr<EAthenaGamePhase>(this);
}

int32 AFortGameStateAthena::GetPlayersLeft() const
{
    static auto PlayersLeft = GetClass()->FindPropertyByName(FName(TEXT("PlayersLeft")));

    return *PlayersLeft->ContainerPtrToValuePtr<int32>(this);
}

const FString& AFortGameStateAthena::GetWinningPlayerName() const
{
    static auto WinningPlayerName = GetClass()->FindPropertyByName(FName(TEXT("WinningPlayerName")));

    return *WinningPlayerName->ContainerPtrToValuePtr<FString>(this);
}
