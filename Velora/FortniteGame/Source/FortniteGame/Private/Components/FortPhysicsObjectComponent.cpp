#include "pch.h"
#include "Components/FortPhysicsObjectComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "Scanner.h"

TMulticastDelegate<void, UFortPhysicsObjectComponent*>& UFortPhysicsObjectComponent::GetNotifyPhysicsObjectComponentActivated()
{
    static auto NotifyPhysicsObjectComponentActivated = Anchor::Data<TMulticastDelegate<void, UFortPhysicsObjectComponent*>*>(TEXT("UFortPhysicsObjectComponent::NotifyPhysicsObjectComponentActivated"))
        .String(TEXT("STAT_FortPhysicsObjectComponent_Activate"))
        .Find("mov rdx, *; lea rcx, [0x*]; call 0x*")
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyPhysicsObjectComponentActivated;
}

TMulticastDelegate<void, UFortPhysicsObjectComponent*>& UFortPhysicsObjectComponent::GetNotifyPhysicsObjectComponentDeactivated()
{
    static auto NotifyPhysicsObjectComponentDeactivated = Anchor::Data<TMulticastDelegate<void, UFortPhysicsObjectComponent*>*>(TEXT("UFortPhysicsObjectComponent::NotifyPhysicsObjectComponentDeactivated"))
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortPhysicsObjectComponent"))
        .Find("mov rdx, *; lea rcx, [0x*]; call 0x*")
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyPhysicsObjectComponentDeactivated;
}

float UFortPhysicsObjectComponent::GetInactivateStateNetUpdateFrequency() const
{
    auto Owner = GetOwner();
    auto Class = Owner ? Owner->GetClass() : nullptr;
    auto DefaultActor = Class ? (AActor*)Class->GetDefaultObject() : nullptr;

    return DefaultActor ? DefaultActor->GetNetUpdateFrequency() : 1.f;
}
