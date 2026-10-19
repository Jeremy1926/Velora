#include "pch.h"
#include "GameplayTagsManager.h"
#include "Scanner.h"

UGameplayTagsManager& UGameplayTagsManager::Get()
{
    static auto SingletonManager = (UGameplayTagsManager**)Scanner::String(TEXT("Gameplay.Effect.InstantDeath.Silent")).Start().Find("mov *, [0x*]; test *").Target(TEXT("UGameplayTagsManager::SingletonManager"));

    if (*SingletonManager == nullptr)
    {
        InitializeManager();
    }

    return **SingletonManager;
}

void UGameplayTagsManager::InitializeManager()
{
    static auto InitializeManager = Scanner::Function<void (*)()>(Scanner::String(TEXT("Gameplay.Effect.InstantDeath.Silent")).Start().Find("mov *, [0x*]; test *").Call().Get(TEXT("UGameplayTagsManager::InitializeManager")));

    InitializeManager();
}

FGameplayTag UGameplayTagsManager::RequestGameplayTag(FName TagName, bool ErrorIfNotFound) const
{
    static auto RequestGameplayTag = Scanner::Function<FGameplayTag* (*)(const UGameplayTagsManager*, FGameplayTag*, FName, bool)>(Scanner::String(TEXT("Gameplay tag not found (marking incompatible): %s")).Last("xor r9d, r9d").Call().Get(TEXT("UGameplayTagsManager::RequestGameplayTag")));

    FGameplayTag Result;

    return *RequestGameplayTag(this, &Result, TagName, ErrorIfNotFound);
}
