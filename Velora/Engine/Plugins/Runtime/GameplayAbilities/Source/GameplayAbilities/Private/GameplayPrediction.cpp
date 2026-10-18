#include "pch.h"
#include "GameplayPrediction.h"
#include "Scanner.h"

FScopedPredictionWindow::FScopedPredictionWindow(UAbilitySystemComponent* AbilitySystemComponent, FPredictionKey InPredictionKey, bool InSetReplicatedPredictionKey)
{
    static auto Constructor = (void (*)(FScopedPredictionWindow*, UAbilitySystemComponent*, FPredictionKey*, bool))Scanner::String(TEXT("Ability %s is overriding pending replicated target data.")).Start().Call().Get(TEXT("FScopedPredictionWindow::FScopedPredictionWindow"));

    Constructor(this, AbilitySystemComponent, &InPredictionKey, InSetReplicatedPredictionKey);
}

FScopedPredictionWindow::~FScopedPredictionWindow()
{
    static auto Destructor = (void (*)(FScopedPredictionWindow*))Scanner::String(TEXT("Ability %s is overriding pending replicated target data.")).Find("ret").Last("call 0x*").Call().Get(TEXT("FScopedPredictionWindow::~FScopedPredictionWindow"));

    Destructor(this);
}
