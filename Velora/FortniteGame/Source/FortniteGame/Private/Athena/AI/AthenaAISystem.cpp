#include "pch.h"
#include "Athena/AI/AthenaAISystem.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UAthenaAISystem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaAISystem"));

    return Class;
}

UAthenaAIPerceptionManager* UAthenaAISystem::GetPerceptionManager() const
{
    static auto PerceptionManager = GetClass()->FindPropertyByName(FName(TEXT("PerceptionManager")));

    return *PerceptionManager->ContainerPtrToValuePtr<UAthenaAIPerceptionManager*>(this);
}
