#include "pch.h"
#include "GameplayPrediction.h"
#include "AbilitySystemComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Actor.h"
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
    static auto ScopedPredictionKey = Anchor::Offset(TEXT("UAbilitySystemComponent::ScopedPredictionKey"))
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("test byte ptr [r*+0x*], 0x20; jz 0x*; mov rax, [r*+0x*]; test rax, rax; jz 0x*; cmp byte ptr [rax+0x*], 0x3")
        .Find("call 0x*; movups xmm0, [r*+0x*]")
        .Find("movups xmm0, [r*+0x*]")
        .Optional()
        .Disp();

    if (ScopedPredictionKey)
    {
        static auto bReplicates = (UBoolProperty*)UActorComponent::StaticClass()->FindPropertyByName(FName(TEXT("bReplicates")));

        Owner = nullptr;
        ClearScopedPredictionKey = false;
        SetReplicatedPredictionKey = false;

        auto OwnerActor = AbilitySystemComponent ? AbilitySystemComponent->GetOwner() : nullptr;

        if (!AbilitySystemComponent || (bReplicates->GetPropertyValue_InContainer(AbilitySystemComponent) && (!OwnerActor || OwnerActor->GetLocalRole() != ROLE_Authority)))
            return;

        auto& CurrentKey = *(FPredictionKey*)((uint8*)AbilitySystemComponent + ScopedPredictionKey);

        Owner = AbilitySystemComponent;
        RestoreKey = CurrentKey;
        CurrentKey = InPredictionKey;
        ClearScopedPredictionKey = true;
        SetReplicatedPredictionKey = InSetReplicatedPredictionKey;

        return;
    }

    static auto Constructor =Anchor::Function<void (*)(FScopedPredictionWindow*, UAbilitySystemComponent*, FPredictionKey*, bool)>(TEXT("FScopedPredictionWindow::FScopedPredictionWindow"))
        .If(!::Anchor::Exists().String(TEXT("Ability %s is overriding pending replicated target data.")).Start().Find("call 0x*").IsTarget("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rbx, rdx; mov rdi, rcx; call 0x*; movzx eax, word ptr [rbx+0x8]").Or().String(TEXT("Ability %s is overriding pending replicated target data.")).Start().Find("call 0x*").IsTarget("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; cmp byte ptr [0x*], 0x0; mov rbx, rcx; mov rcx, [rdx]; mov rdi, rdx; mov [rbx], rcx; jnz 0x*; movzx eax, word ptr [rdi+0x8]").Get())
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Call()
        .Or()
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("call 0x*")
        .IsTarget("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rbx, rdx; mov rdi, rcx; call 0x*; movzx eax, word ptr [rbx+0x8]")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("call 0x*")
        .IsTarget("mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; cmp byte ptr [0x*], 0x0; mov rbx, rcx; mov rcx, [rdx]; mov rdi, rdx; mov [rbx], rcx; jnz 0x*; movzx eax, word ptr [rdi+0x8]")
        .Find("call 0x*")
        .Target()
        .Get();

    Constructor(this, AbilitySystemComponent, &InPredictionKey, InSetReplicatedPredictionKey);
}

FScopedPredictionWindow::~FScopedPredictionWindow()
{
    static auto ScopedPredictionKey = Anchor::Offset(TEXT("UAbilitySystemComponent::ScopedPredictionKey"))
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("test byte ptr [r*+0x*], 0x20; jz 0x*; mov rax, [r*+0x*]; test rax, rax; jz 0x*; cmp byte ptr [rax+0x*], 0x3")
        .Find("call 0x*; movups xmm0, [r*+0x*]")
        .Find("movups xmm0, [r*+0x*]")
        .Optional()
        .Disp();

    if (ScopedPredictionKey)
    {
        static auto ReplicatedPredictionKeyMap = UAbilitySystemComponent::StaticClass()->FindPropertyByName(FName(TEXT("ReplicatedPredictionKeyMap")));
        static auto KeyMapStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.ReplicatedPredictionKeyMap"));
        static auto KeyItemStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.ReplicatedPredictionKeyItem"));
        static auto PredictionKeys = KeyMapStruct->FindPropertyByName(FName(TEXT("PredictionKeys")));
        static auto ItemPredictionKey = KeyItemStruct->FindPropertyByName(FName(TEXT("PredictionKey")));

        auto OwnerPtr = Owner.Get();

        if (!OwnerPtr)
            return;

        auto& CurrentKey = *(FPredictionKey*)((uint8*)OwnerPtr + ScopedPredictionKey);

        if (SetReplicatedPredictionKey && CurrentKey.IsValidKey())
        {
            auto KeyMap = ReplicatedPredictionKeyMap->ContainerPtrToValuePtr<FFastArraySerializer>(OwnerPtr);
            auto& Items = *PredictionKeys->ContainerPtrToValuePtr<TArray<uint8>>(KeyMap);
            auto Item = Items.GetData() + (CurrentKey.GetCurrent() % 32) * KeyItemStruct->GetPropertiesSize();

            *ItemPredictionKey->ContainerPtrToValuePtr<FPredictionKey>(Item) = CurrentKey;
            KeyMap->MarkItemDirty(*(FFastArraySerializerItem*)Item);
        }

        if (ClearScopedPredictionKey)
            CurrentKey = RestoreKey;

        return;
    }

    static auto Destructor =Anchor::Function<void (*)(FScopedPredictionWindow*)>(TEXT("FScopedPredictionWindow::~FScopedPredictionWindow"))
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
