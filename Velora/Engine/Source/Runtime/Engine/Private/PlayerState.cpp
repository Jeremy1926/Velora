#include "pch.h"
#include "GameFramework/PlayerState.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

const FUniqueNetIdRepl& APlayerState::GetUniqueId() const
{
    static auto UniqueId = GetClass()->FindPropertyByName(FName(TEXT("UniqueId")));

    return *UniqueId->ContainerPtrToValuePtr<FUniqueNetIdRepl>(this);
}
