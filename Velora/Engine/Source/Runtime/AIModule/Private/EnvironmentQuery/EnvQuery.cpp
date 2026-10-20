#include "pch.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "UObject/UObjectGlobals.h"

UClass* UEnvQuery::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.EnvQuery"));

    return Class;
}
