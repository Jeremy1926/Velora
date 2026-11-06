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
        .If(!::Anchor::Exists().String(TEXT("Ability %s is overriding pending replicated target data.")).Start().Find("call 0x*").IsTarget("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rbx, rdx; mov rdi, rcx; call 0x*; movzx eax, word ptr [rbx+0x8]").Get())
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Call()
        .Or()
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("call 0x*")
        .IsTarget("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rbx, rdx; mov rdi, rcx; call 0x*; movzx eax, word ptr [rbx+0x8]")
        .Find("call 0x*", 2)
        .Target()
        .Get();

    Constructor(this, AbilitySystemComponent, &InPredictionKey, InSetReplicatedPredictionKey);
}

FScopedPredictionWindow::~FScopedPredictionWindow()
{
    static auto Destructor = Anchor::Function<void (*)(FScopedPredictionWindow*)>(TEXT("FScopedPredictionWindow::~FScopedPredictionWindow"))
        .If(!::Anchor::Exists().String(TEXT("Ability %s is overriding pending replicated target data.")).Find("ret").Last("call 0x*").IsTarget("cmp rcx, [0x*]").Get())
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Or()
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Find("ret")
        .Last("xor rcx, rsp; call 0x*")
        .Last("call 0x*")
        .Call()
        .Get();

    Destructor(this);
}
