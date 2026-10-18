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
