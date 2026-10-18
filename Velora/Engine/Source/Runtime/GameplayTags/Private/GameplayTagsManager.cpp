#include "pch.h"
#include "GameplayTagsManager.h"
#include "Scanner.h"

UGameplayTagsManager& UGameplayTagsManager::Get()
{
    static auto SingletonManager = (UGameplayTagsManager**)Scanner::String(TEXT("Gameplay.Effect.InstantDeath.Silent")).Start().Find("mov *, [0x*]; test *").Target();

    if (*SingletonManager == nullptr)
    {
        InitializeManager();
    }

    return **SingletonManager;
}

void UGameplayTagsManager::InitializeManager()
{
    static auto InitializeManager = (void (*)())Scanner::String(TEXT("Gameplay.Effect.InstantDeath.Silent")).Start().Find("mov *, [0x*]; test *").Call().Get();

    InitializeManager();
}

FGameplayTag UGameplayTagsManager::RequestGameplayTag(FName TagName, bool ErrorIfNotFound) const
{
    static auto RequestGameplayTag = (FGameplayTag* (*)(const UGameplayTagsManager*, FGameplayTag*, FName, bool))Scanner::String(TEXT("Gameplay.Effect.InstantDeath.Silent")).Call(2).Get();

    FGameplayTag Result;

    return *RequestGameplayTag(this, &Result, TagName, ErrorIfNotFound);
}
