#include "pch.h"
#include "FortGameState.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortGameState::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameState"));

    return Class;
}

EFortGameplayState AFortGameState::GetGameplayState() const
{
    static auto GameplayState = GetClass()->FindPropertyByName(FName(TEXT("GameplayState")));

    return *GameplayState->ContainerPtrToValuePtr<EFortGameplayState>(this);
}
