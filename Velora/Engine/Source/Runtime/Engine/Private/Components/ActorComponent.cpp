#include "pch.h"
#include "Components/ActorComponent.h"
#include "Scanner.h"

AActor* UActorComponent::GetOwner() const
{
    static auto OwnerPrivate = Scanner::Exec(TEXT("/Script/Engine.ActorComponent:GetOwner")).Find("mov rax, [rcx+0x*]").Disp(TEXT("UActorComponent::OwnerPrivate"));

    return *(AActor**)((uint8_t*)this + OwnerPrivate);
}
