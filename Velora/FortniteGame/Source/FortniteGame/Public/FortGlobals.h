#pragma once

#include "UObject/Object.h"
#include "Containers/UnrealString.h"

class UFortGameFeaturePluginManager;
class IAbilitySystemInterface;

class UFortGlobals : public UObject
{
public:
    static UClass* StaticClass();

    static UFortGlobals* Get();

    static bool HasInitializePlayerGameplayAbilities();

    static void InitializePlayerGameplayAbilities(IAbilitySystemInterface* PlayerStateOrProxy);

    FString* GetBRMapFullName();

    UFortGameFeaturePluginManager* GetGameFeaturePluginManager() const;
};
