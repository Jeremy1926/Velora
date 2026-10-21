#include "pch.h"
#include "GameplayAbilitySpec.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

FGameplayAbilitySpec::FGameplayAbilitySpec(UGameplayAbility* InAbility, int32 InLevel, int32 InInputID, UObject* InSourceObject)
{
    FMemory::Memzero(this, sizeof(FGameplayAbilitySpec));

    StaticStruct()->InitializeStruct(this);

    Ability = InAbility;
    Level = InLevel;
    InputID = InInputID;
    SourceObject = InSourceObject;

    Handle.GenerateNewHandle();
}

FGameplayAbilitySpec::~FGameplayAbilitySpec()
{
    StaticStruct()->DestroyStruct(this);
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
        .Get();

    Handle = GHandle++;
}
