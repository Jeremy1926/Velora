#include "pch.h"
#include "GameplayAbilitySpec.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

FGameplayAbilitySpec::FGameplayAbilitySpec(UGameplayAbility* InAbility, int32 InLevel, int32 InInputID, UObject* InSourceObject)
{
    static auto SourceObjectProperty = StaticStruct()->FindPropertyByName(FName(TEXT("SourceObject")));
    static auto bWeakSourceObject = SourceObjectProperty && SourceObjectProperty->IsWeakObjectProperty();

    FMemory::Memzero(this, sizeof(FGameplayAbilitySpec));

    StaticStruct()->InitializeStruct(this);

    Ability = InAbility;
    Level = InLevel;
    InputID = InInputID;

    if (bWeakSourceObject)
        *SourceObjectProperty->ContainerPtrToValuePtr<FWeakObjectPtr>(this) = InSourceObject;
    else
        SourceObject = InSourceObject;

    Handle.GenerateNewHandle();
}

FGameplayAbilitySpec::~FGameplayAbilitySpec()
{
    StaticStruct()->DestroyStruct(this);
}

UGameplayAbility* FGameplayAbilitySpec::GetPrimaryInstance() const
{
    static auto NonReplicatedInstances = StaticStruct()->FindPropertyByName(FName(TEXT("NonReplicatedInstances")));
    static auto ReplicatedInstances = StaticStruct()->FindPropertyByName(FName(TEXT("ReplicatedInstances")));

    for (auto Instances : { NonReplicatedInstances, ReplicatedInstances })
    {
        if (Instances && Instances->ContainerPtrToValuePtr<TArray<UGameplayAbility*>>(this)->Num() > 0)
            return (*Instances->ContainerPtrToValuePtr<TArray<UGameplayAbility*>>(this))[0];
    }

    return nullptr;
}

TArray<UGameplayAbility*> FGameplayAbilitySpec::GetAbilityInstances() const
{
    static auto NonReplicatedInstances = StaticStruct()->FindPropertyByName(FName(TEXT("NonReplicatedInstances")));
    static auto ReplicatedInstances = StaticStruct()->FindPropertyByName(FName(TEXT("ReplicatedInstances")));

    TArray<UGameplayAbility*> Abilities;

    for (auto Instances : { ReplicatedInstances, NonReplicatedInstances })
    {
        if (!Instances)
            continue;

        for (auto Instance : *Instances->ContainerPtrToValuePtr<TArray<UGameplayAbility*>>(this))
            Abilities.Add(Instance);
    }

    return Abilities;
}

UScriptStruct* FGameplayAbilitySpec::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.GameplayAbilitySpec"));

    return Struct;
}

int32 FGameplayAbilitySpec::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

UScriptStruct* FGameplayAbilitySpecContainer::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.GameplayAbilitySpecContainer"));

    return Struct;
}

TRuntimeSizedArray<FGameplayAbilitySpec>& FGameplayAbilitySpecContainer::GetItems()
{
    static auto Items = StaticStruct()->FindPropertyByName(FName(TEXT("Items")));

    return *Items->ContainerPtrToValuePtr<TRuntimeSizedArray<FGameplayAbilitySpec>>(this);
}

void FGameplayAbilitySpecHandle::GenerateNewHandle()
{
    static auto& GHandle = *Anchor::Data<int32*>(TEXT("FGameplayAbilitySpecHandle::GenerateNewHandle::GHandle"))
        .String(TEXT("FGameplayAbilitySpec::FGameplayAbilitySpec for %s from %s"))
        .Start()
        .Find("mov eax, [0x*]")
        .Target()
        .Or()
        .String(TEXT("FGameplayAbilitySpec::FGameplayAbilitySpec for %s from %s"))
        .Start()
        .Find("mov edx, [0x*]")
        .Target()
        .Get();

    Handle = GHandle++;
}
