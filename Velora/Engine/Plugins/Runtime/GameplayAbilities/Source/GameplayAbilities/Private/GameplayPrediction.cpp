#include "pch.h"
#include "GameplayPrediction.h"
#include "Scanner.h"

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
