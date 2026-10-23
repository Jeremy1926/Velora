#include "pch.h"
#include "GameFramework/WorldSettings.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FNetViewer::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.NetViewer"));

    return Struct;
}

int32 FNetViewer::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FVector FNetViewer::GetViewLocation() const
{
    static auto ViewLocation = StaticStruct()->FindPropertyByName(FName(TEXT("ViewLocation")));

    return ViewLocation->GetPropertyValue_InContainer<FVector>(this);
}

FVector FNetViewer::GetViewDir() const
{
    static auto ViewDir = StaticStruct()->FindPropertyByName(FName(TEXT("ViewDir")));

    return ViewDir->GetPropertyValue_InContainer<FVector>(this);
}

TArray<FNetViewer>& AWorldSettings::GetReplicationViewers()
{
    static auto ReplicationViewers = GetClass()->FindPropertyByName(FName(TEXT("ReplicationViewers")));

    return *ReplicationViewers->ContainerPtrToValuePtr<TArray<FNetViewer>>(this);
}
