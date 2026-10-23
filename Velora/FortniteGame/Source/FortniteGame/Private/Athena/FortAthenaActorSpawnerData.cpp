#include "pch.h"
#include "Athena/FortAthenaActorSpawnerData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortAthenaActorSpawnerData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaActorSpawnerData"));

    return Class;
}

UClass* UFortAthenaActorSpawnerData::GetActorClass()
{
    static auto ActorClass = GetClass()->FindPropertyByName(FName(TEXT("ActorClass")));

    return *ActorClass->ContainerPtrToValuePtr<UClass*>(this);
}

FGameplayTagContainer& UFortAthenaActorSpawnerData::GetDescriptorTag()
{
    static auto DescriptorTag = GetClass()->FindPropertyByName(FName(TEXT("DescriptorTag")));

    return *DescriptorTag->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}
