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
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAISpawnerData:CreateComponentListFromClass"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto CreateComponentList = Anchor::Function<UFortAthenaAISpawnerDataComponentList* (*)(UFortAthenaAISpawnerData*, UObject*)>(TEXT("UFortAthenaAISpawnerData::CreateComponentList"))
            .Exec(TEXT("/Script/FortniteGame.FortAthenaAISpawnerData:CreateComponentListFromClass"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("mov rcx, rax; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get();

        auto SpawnerData = AISpawnerDataClass ? (UFortAthenaAISpawnerData*)AISpawnerDataClass->GetDefaultObject() : nullptr;

        return SpawnerData ? CreateComponentList(SpawnerData, OuterObject) : nullptr;
    }

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
