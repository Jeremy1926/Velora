#include "pch.h"
#include "GameplayTagsManager.h"
#include "Scanner.h"

UGameplayTagsManager& UGameplayTagsManager::Get()
{
    static auto SingletonManager = Anchor::Data<UGameplayTagsManager**>(TEXT("UGameplayTagsManager::SingletonManager"))
        .String(TEXT("Gameplay.Effect.InstantDeath.Silent"))
        .Start()
        .Find("mov *, [0x*]; test *")
        .Target()
        .Or()
        .String("Gameplay.Effect.InstantDeath.Silent")
        .Start()
        .Find("mov *, [0x*]; test *")
        .Target()
        .Get();

    if (*SingletonManager == nullptr)
    {
        InitializeManager();
    }

    return **SingletonManager;
}

void UGameplayTagsManager::InitializeManager()
{
    static auto InitializeManager = Anchor::Function<void (*)()>(TEXT("UGameplayTagsManager::InitializeManager"))
        .String(TEXT("Gameplay.Effect.InstantDeath.Silent"))
        .Start()
        .Find("mov *, [0x*]; test *")
        .Call()
        .Or()
        .String("Gameplay.Effect.InstantDeath.Silent")
        .Start()
        .Find("mov *, [0x*]; test *")
        .Call()
        .Get();

    InitializeManager();
}

FGameplayTag UGameplayTagsManager::RequestGameplayTag(FName TagName, bool ErrorIfNotFound) const
{
    static auto RequestGameplayTag = Anchor::Function<FGameplayTag* (*)(const UGameplayTagsManager*, FGameplayTag*, FName, bool)>(TEXT("UGameplayTagsManager::RequestGameplayTag"))
        .String(TEXT("Gameplay tag not found (marking incompatible): %s"))
        .Last("xor r9d, r9d")
        .Call()
        .Get();

    FGameplayTag Result;

    return *RequestGameplayTag(this, &Result, TagName, ErrorIfNotFound);
}
