#include "pch.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UActorComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.ActorComponent"));

    return Class;
}

AActor* UActorComponent::GetOwner() const
{
    static auto OwnerPrivate = Anchor::Offset(TEXT("UActorComponent::OwnerPrivate"))
        .Exec(TEXT("/Script/Engine.ActorComponent:GetOwner"))
        .Find("mov rax, [rcx+0x*]")
        .Disp();

    return *(AActor**)((uint8_t*)this + OwnerPrivate);
}

void UActorComponent::RegisterComponentTickFunctions(bool bRegister)
{
    static auto RegisterComponentTickFunctions = Anchor::VirtualFunction<void (*)(UActorComponent*, bool)>(TEXT("UActorComponent::RegisterComponentTickFunctions"))
        .FieldUserIn(TEXT("/Script/Engine.Default__ActorComponent"), "add rcx, %x; test dl, dl", StaticClass()->FindPropertyByName(FName(TEXT("PrimaryComponentTick"))))
        .Or()
        .FieldUserIn(TEXT("/Script/Engine.Default__ActorComponent"), "lea r*, [rcx+%x]; mov r*, rcx; test dl, dl", StaticClass()->FindPropertyByName(FName(TEXT("PrimaryComponentTick"))))
        .Or()
        .FieldUserIn(TEXT("/Script/Engine.Default__ActorComponent"), "lea r*, [rcx+%x]; xor e*, e*; test dl, dl", StaticClass()->FindPropertyByName(FName(TEXT("PrimaryComponentTick"))))
        .Get();

    RegisterComponentTickFunctions(this, bRegister);
}

FActorComponentTickFunction& UActorComponent::GetPrimaryComponentTick()
{
    static auto PrimaryComponentTick = StaticClass()->FindPropertyByName(FName(TEXT("PrimaryComponentTick")));

    return *PrimaryComponentTick->ContainerPtrToValuePtr<FActorComponentTickFunction>(this);
}

UScriptStruct* FTickFunction::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.TickFunction"));

    return Struct;
}

bool FTickFunction::GetbCanEverTick() const
{
    static auto bCanEverTick = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bCanEverTick")));

    return bCanEverTick->GetPropertyValue_InContainer(this);
}

void FTickFunction::SetbCanEverTick(bool bInCanEverTick)
{
    static auto bCanEverTick = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bCanEverTick")));

    bCanEverTick->SetPropertyValue_InContainer(this, bInCanEverTick);
}

void FTickFunction::SetbStartWithTickEnabled(bool bInStartWithTickEnabled)
{
    static auto bStartWithTickEnabled = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bStartWithTickEnabled")));

    bStartWithTickEnabled->SetPropertyValue_InContainer(this, bInStartWithTickEnabled);
}
