#include "pch.h"
#include "GameplayPrediction.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

FPredictionKey::FPredictionKey()
{
    FMemory::Memzero(this, sizeof(FPredictionKey));
}

UScriptStruct* FPredictionKey::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.PredictionKey"));

    return Struct;
}

FPredictionKey::KeyType FPredictionKey::GetCurrent() const
{
    static auto Current = StaticStruct()->FindPropertyByName(FName(TEXT("Current")));

    return *Current->ContainerPtrToValuePtr<KeyType>(this);
}

FScopedPredictionWindow::FScopedPredictionWindow(UAbilitySystemComponent* AbilitySystemComponent, FPredictionKey InPredictionKey, bool InSetReplicatedPredictionKey)
{
    static auto Constructor = Anchor::Function<void (*)(FScopedPredictionWindow*, UAbilitySystemComponent*, FPredictionKey*, bool)>(TEXT("FScopedPredictionWindow::FScopedPredictionWindow"))
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Call()
        .Get();

    Constructor(this, AbilitySystemComponent, &InPredictionKey, InSetReplicatedPredictionKey);
}

FScopedPredictionWindow::~FScopedPredictionWindow()
{
    static auto Destructor = Anchor::Function<void (*)(FScopedPredictionWindow*)>(TEXT("FScopedPredictionWindow::~FScopedPredictionWindow"))
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Get();

    Destructor(this);
}
