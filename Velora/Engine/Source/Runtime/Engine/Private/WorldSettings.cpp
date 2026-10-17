#include "pch.h"
#include "GameFramework/WorldSettings.h"
#include "UObject/UnrealType.h"

TArray<FNetViewer>& AWorldSettings::GetReplicationViewers()
{
    static auto ReplicationViewers = GetClass()->FindPropertyByName(FName(TEXT("ReplicationViewers")));

    return *ReplicationViewers->ContainerPtrToValuePtr<TArray<FNetViewer>>(this);
}
