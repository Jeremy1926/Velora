#include "pch.h"
#include "Athena/AI/FortAthenaAISpawnerData.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortAthenaAISpawnerData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAISpawnerData"));

    return Class;
}

UFortAthenaAISpawnerDataComponentList* UFortAthenaAISpawnerData::CreateComponentListFromClass(UClass* AISpawnerDataClass, UObject* OuterObject)
{
    static auto CreateComponentListFromClass = Anchor::Function<UFortAthenaAISpawnerDataComponentList* (*)(UClass*, UObject*)>(TEXT("UFortAthenaAISpawnerData::CreateComponentListFromClass"))
        .Native(TEXT("/Script/FortniteGame.FortAthenaAISpawnerData:CreateComponentListFromClass"))
        .Get();

    return CreateComponentListFromClass(AISpawnerDataClass, OuterObject);
}

FGameplayTagContainer& UFortAthenaAISpawnerData::GetDescriptorTag()
{
    static auto DescriptorTag = GetClass()->FindPropertyByName(FName(TEXT("DescriptorTag")));

    return *DescriptorTag->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}
