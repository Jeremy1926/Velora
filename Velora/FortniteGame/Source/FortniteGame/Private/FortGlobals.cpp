#include "pch.h"
#include "FortGlobals.h"
#include "Engine/Engine.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortGlobals::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGlobals"));

    return Class;
}

UFortGlobals* UFortGlobals::Get()
{
    return Cast<UFortGlobals>(GEngine->GetGameSingleton());
}

bool UFortGlobals::HasInitializePlayerGameplayAbilities()
{
    static auto bHasInitializePlayerGameplayAbilities = Anchor::Exists()
        .String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!"))
        .Start()
        .Get();

    return bHasInitializePlayerGameplayAbilities;
}

void UFortGlobals::InitializePlayerGameplayAbilities(IAbilitySystemInterface* PlayerStateOrProxy)
{
    static auto InitializePlayerGameplayAbilities = Anchor::Function<void (*)(IAbilitySystemInterface*)>(TEXT("UFortGlobals::InitializePlayerGameplayAbilities"))
        .String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!"))
        .Start()
        .Get();

    InitializePlayerGameplayAbilities(PlayerStateOrProxy);
}

FString* UFortGlobals::GetBRMapFullName()
{
    static auto BRMapFullName = GetClass()->FindPropertyByName(FName(TEXT("BRMapFullName")));

    return BRMapFullName ? BRMapFullName->ContainerPtrToValuePtr<FString>(this) : nullptr;
}

UFortGameFeaturePluginManager* UFortGlobals::GetGameFeaturePluginManager() const
{
    static auto GameFeaturePluginManager = GetClass()->FindPropertyByName(FName(TEXT("GameFeaturePluginManager")));

    return GameFeaturePluginManager ? *GameFeaturePluginManager->ContainerPtrToValuePtr<UFortGameFeaturePluginManager*>(this) : nullptr;
}
