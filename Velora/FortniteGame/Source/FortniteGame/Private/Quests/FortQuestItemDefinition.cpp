#include "pch.h"
#include "Quests/FortQuestItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortQuestItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuestItemDefinition"));

    return Class;
}

const TArray<TSoftObjectPtr<UFortQuestItemDefinition>>& UFortQuestItemDefinition::GetTransientPrerequisiteQuests() const
{
    static auto TransientPrerequisiteQuests = StaticClass()->FindPropertyByName(FName(TEXT("TransientPrerequisiteQuests")));

    return *TransientPrerequisiteQuests->ContainerPtrToValuePtr<TArray<TSoftObjectPtr<UFortQuestItemDefinition>>>(this);
}
