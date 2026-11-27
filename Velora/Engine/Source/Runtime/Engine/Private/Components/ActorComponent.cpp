#include "pch.h"
#include "Components/ActorComponent.h"
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

FActorComponentTickFunction& UActorComponent::GetPrimaryComponentTick()
{
    static auto PrimaryComponentTick = StaticClass()->FindPropertyByName(FName(TEXT("PrimaryComponentTick")));

    return *PrimaryComponentTick->ContainerPtrToValuePtr<FActorComponentTickFunction>(this);
}
