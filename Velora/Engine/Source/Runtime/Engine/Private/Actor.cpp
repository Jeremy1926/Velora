#include "pch.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Actor"));

    return Class;
}

bool AActor::GetOnlyRelevantToOwner() const
{
    static auto bOnlyRelevantToOwner = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bOnlyRelevantToOwner")));

    return bOnlyRelevantToOwner->GetPropertyValue_InContainer(this);
}

bool AActor::GetTearOff() const
{
    static auto bTearOff = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bTearOff")));

    return bTearOff->GetPropertyValue_InContainer(this);
}

bool AActor::IsNetStartupActor() const
{
    static auto bNetStartup = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bNetStartup")));

    return bNetStartup->GetPropertyValue_InContainer(this);
}

ENetRole AActor::GetRemoteRole() const
{
    static auto RemoteRole = GetClass()->FindPropertyByName(FName(TEXT("RemoteRole")));

    return (ENetRole)*RemoteRole->ContainerPtrToValuePtr<uint8>(this);
}

ENetRole AActor::GetLocalRole() const
{
    static auto Role = GetClass()->FindPropertyByName(FName(TEXT("Role")));

    return (ENetRole)*Role->ContainerPtrToValuePtr<uint8>(this);
}

bool AActor::HasAuthority() const
{
    static auto Role = GetClass()->FindPropertyByName(FName(TEXT("Role")));

    return (ENetRole)*Role->ContainerPtrToValuePtr<uint8>(this) == ROLE_Authority;
}

bool AActor::CanBeDamaged() const
{
    static auto bCanBeDamaged = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bCanBeDamaged")));

    return bCanBeDamaged->GetPropertyValue_InContainer(this);
}

FVector AActor::GetActorForwardVector() const
{
    return GetActorRotation().Vector();
}

void AActor::SetLifeSpan(float InLifespan)
{
    static auto SetLifeSpan = Anchor::VirtualFunction<void (*)(AActor*, float)>(TEXT("AActor::SetLifeSpan"))
        .Exec(TEXT("/Script/Engine.Actor:SetLifeSpan"))
        .Find("call [*+0x*]")
        .Get();

    SetLifeSpan(this, InLifespan);
}

bool AActor::TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest, bool bNoCheck)
{
    static auto TeleportTo = Anchor::VirtualFunction<bool (*)(AActor*, const TGameValue<FVector>&, const TGameValue<FRotator>&, bool, bool)>(TEXT("AActor::TeleportTo"))
        .Exec(TEXT("/Script/Engine.Actor:K2_TeleportTo"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Get();

    return TeleportTo(this, DestLocation, DestRotation, bIsATest, bNoCheck);
}

void AActor::K2_AttachToActor(AActor* ParentActor, FName SocketName, EAttachmentRule LocationRule, EAttachmentRule RotationRule, EAttachmentRule ScaleRule, bool bWeldSimulatedBodies)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/Engine.Actor:K2_AttachToActor"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto GetDefaultAttachComponent = Anchor::VirtualFunction<USceneComponent* (*)(const AActor*)>(TEXT("AActor::GetDefaultAttachComponent"))
            .Exec(TEXT("/Script/Engine.Actor:K2_AttachToActor"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [*+0x*]")
            .Get();

        static auto AttachToComponent = Anchor::Function<bool (*)(USceneComponent*, USceneComponent*, const FAttachmentTransformRules*, FName)>(TEXT("USceneComponent::AttachToComponent"))
            .Exec(TEXT("/Script/Engine.Actor:K2_AttachToActor"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [*+0x*]")
            .Find("call 0x*")
            .Target()
            .Get();

        const FAttachmentTransformRules AttachmentRules(LocationRule, RotationRule, ScaleRule, bWeldSimulatedBodies);
        auto RootComponent = GetRootComponent();

        if (RootComponent && ParentActor)
        {
            if (auto ParentDefaultAttachComponent = GetDefaultAttachComponent(ParentActor))
                AttachToComponent(RootComponent, ParentDefaultAttachComponent, &AttachmentRules, SocketName);
        }

        return;
    }

    static auto K2_AttachToActor = Anchor::Function<void (*)(AActor*, AActor*, FName, EAttachmentRule, EAttachmentRule, EAttachmentRule, bool)>(TEXT("AActor::K2_AttachToActor"))
        .Native(TEXT("/Script/Engine.Actor:K2_AttachToActor"))
        .Get();

    K2_AttachToActor(this, ParentActor, SocketName, LocationRule, RotationRule, ScaleRule, bWeldSimulatedBodies);
}

void AActor::K2_DetachFromActor(EDetachmentRule LocationRule, EDetachmentRule RotationRule, EDetachmentRule ScaleRule)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/Engine.Actor:K2_DetachFromActor"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto DetachFromComponent = Anchor::VirtualFunction<void (*)(USceneComponent*, const FDetachmentTransformRules*)>(TEXT("USceneComponent::DetachFromComponent"))
            .Exec(TEXT("/Script/Engine.Actor:K2_DetachFromActor"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [*+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.Actor:K2_DetachFromActor"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Find("call [*+0x*]")
            .Get();

        const FDetachmentTransformRules DetachmentRules(LocationRule, RotationRule, ScaleRule, true);

        if (auto RootComponent = GetRootComponent())
            DetachFromComponent(RootComponent, &DetachmentRules);

        return;
    }

    static auto K2_DetachFromActor = Anchor::Function<void (*)(AActor*, EDetachmentRule, EDetachmentRule, EDetachmentRule)>(TEXT("AActor::K2_DetachFromActor"))
        .Native(TEXT("/Script/Engine.Actor:K2_DetachFromActor"))
        .Get();

    K2_DetachFromActor(this, LocationRule, RotationRule, ScaleRule);
}

ENetDormancy AActor::GetNetDormancy() const
{
    static auto NetDormancy = GetClass()->FindPropertyByName(FName(TEXT("NetDormancy")));

    return (ENetDormancy)*NetDormancy->ContainerPtrToValuePtr<uint8>(this);
}

float AActor::GetNetUpdateFrequency() const
{
    static auto NetUpdateFrequency = GetClass()->FindPropertyByName(FName(TEXT("NetUpdateFrequency")));

    return *NetUpdateFrequency->ContainerPtrToValuePtr<float>(this);
}

float AActor::GetNetCullDistanceSquared() const
{
    static auto NetCullDistanceSquared = GetClass()->FindPropertyByName(FName(TEXT("NetCullDistanceSquared")));

    return *NetCullDistanceSquared->ContainerPtrToValuePtr<float>(this);
}

float AActor::GetMinNetUpdateFrequency() const
{
    static auto MinNetUpdateFrequency = GetClass()->FindPropertyByName(FName(TEXT("MinNetUpdateFrequency")));

    return *MinNetUpdateFrequency->ContainerPtrToValuePtr<float>(this);
}

void AActor::SetMinNetUpdateFrequency(float InMinNetUpdateFrequency)
{
    static auto MinNetUpdateFrequency = GetClass()->FindPropertyByName(FName(TEXT("MinNetUpdateFrequency")));

    *MinNetUpdateFrequency->ContainerPtrToValuePtr<float>(this) = InMinNetUpdateFrequency;
}

int32 AActor::GetNetTag() const
{
    static auto NetTag = GetClass()->FindPropertyByName(FName(TEXT("NetTag")));

    return *NetTag->ContainerPtrToValuePtr<int32>(this);
}

void AActor::SetNetTag(int32 InNetTag)
{
    static auto NetTag = GetClass()->FindPropertyByName(FName(TEXT("NetTag")));

    *NetTag->ContainerPtrToValuePtr<int32>(this) = InNetTag;
}

FName AActor::GetNetDriverName() const
{
    static auto NetDriverName = GetClass()->FindPropertyByName(FName(TEXT("NetDriverName")));

    return NetDriverName->GetPropertyValue_InContainer<FName>(this);
}

UNetDriver* AActor::GetNetDriver() const
{
    auto World = GetWorld();

    if (GetNetDriverName() == NAME_GameNetDriver)
        return World ? World->GetNetDriver() : nullptr;

    return GEngine->FindNamedNetDriver(World, GetNetDriverName());
}

ENetMode AActor::InternalGetNetMode() const
{
    if (auto NetDriver = GetNetDriver())
        return NetDriver->GetNetMode();

    auto World = GetWorld();

    if (World && World->GetDemoNetDriver())
        return World->GetDemoNetDriver()->GetNetMode();

    return NM_Standalone;
}

bool AActor::IsNetMode(ENetMode Mode) const
{
    if (Mode == NM_DedicatedServer)
        return false;

    return InternalGetNetMode() == Mode;
}

USceneComponent* AActor::GetRootComponent() const
{
    static auto RootComponent = GetClass()->FindPropertyByName(FName(TEXT("RootComponent")));

    return *RootComponent->ContainerPtrToValuePtr<USceneComponent*>(this);
}

APawn* AActor::GetInstigator() const
{
    static auto Instigator = GetClass()->FindPropertyByName(FName(TEXT("Instigator")));

    return *Instigator->ContainerPtrToValuePtr<APawn*>(this);
}

bool AActor::IsPendingKillPending() const
{
    static auto Offset = Anchor::Offset(TEXT("AActor::bActorIsBeingDestroyed"))
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Find("test byte ptr [*], 0x*")
        .Or()
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Find("cmp byte ptr [*], 0x80")
        .Or()
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Start()
        .Find("or byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Start()
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [*+0x*], 0x*")
        .Disp();

    static auto Mask = Anchor::Offset(TEXT("AActor::bActorIsBeingDestroyed"))
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Find("test byte ptr [*], 0x*")
        .Or()
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Find("cmp byte ptr [*], 0x80")
        .Or()
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Start()
        .Find("or byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("Destroying %s, which doesn't have a valid world pointer"))
        .Start()
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [*+0x*], 0x*")
        .Value();

    return (*((uint8_t*)this + Offset) & Mask) || IsPendingKill();
}

bool AActor::IsActorInitialized() const
{
    static auto Offset = Anchor::Offset(TEXT("AActor::bActorInitialized"))
        .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
        .Last("test byte ptr [*], 0x*")
        .Or()
        .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
        .Start()
        .Find("test [*+0x8], *; jnz *; test byte ptr [*], 0x*")
        .Find("test byte ptr [*], 0x*")
        .Disp();

    static auto Mask = Anchor::Offset(TEXT("AActor::bActorInitialized"))
        .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
        .Last("test byte ptr [*], 0x*")
        .Or()
        .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
        .Start()
        .Find("test [*+0x8], *; jnz *; test byte ptr [*], 0x*")
        .Find("test byte ptr [*], 0x*")
        .Value();

    return *((uint8_t*)this + Offset) & Mask;
}

AActor* AActor::GetOwner() const
{
    static auto Owner = GetClass()->FindPropertyByName(FName(TEXT("Owner")));

    return *Owner->ContainerPtrToValuePtr<AActor*>(this);
}

FActorDestroyedSignature& AActor::GetOnDestroyed()
{
    static auto OnDestroyed = GetClass()->FindPropertyByName(FName(TEXT("OnDestroyed")));

    return *OnDestroyed->ContainerPtrToValuePtr<FActorDestroyedSignature>(this);
}

UActorComponent* AActor::FindComponentByClass(const TSubclassOf<UActorComponent> ComponentClass) const
{
    static auto FindComponentByClass = Anchor::VirtualFunction<UActorComponent* (*)(const AActor*, const TSubclassOf<UActorComponent>)>(TEXT("AActor::FindComponentByClass"))
        .Native(TEXT("/Script/Engine.Actor:GetComponentByClass"))
        .Find("jmp [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.Actor:GetComponentByClass"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Get();

    return FindComponentByClass(this, ComponentClass);
}

ULevel* AActor::GetLevel() const
{
    return (ULevel*)GetOuter();
}

const AActor* AActor::GetNetOwner() const
{
    static auto GetNetOwner = Anchor::VirtualFunction<const AActor* (*)(const AActor*)>(TEXT("AActor::GetNetOwner"))
        .FieldVirtual(TEXT("/Script/Engine.Default__Actor"), "mov rax, [rcx+%x]; ret", GetClass()->FindPropertyByName(FName(TEXT("Owner"))))
        .Get();

    return GetNetOwner(this);
}

bool AActor::IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const
{
    static auto IsRelevancyOwnerFor = Anchor::VirtualFunction<bool (*)(const AActor*, const AActor*, const AActor*, const AActor*)>(TEXT("AActor::IsRelevancyOwnerFor"))
        .VirtualMatch(TEXT("/Script/Engine.Default__Actor"), "cmp r8, rcx; setz al; ret")
        .Get();

    return IsRelevancyOwnerFor(this, ReplicatedActor, ActorOwner, ConnectionActor);
}

float AActor::GetDistanceTo(const AActor* OtherActor) const
{
    static auto GetDistanceTo = Anchor::Function<float (*)(const AActor*, const AActor*)>(TEXT("AActor::GetDistanceTo"))
        .Native(TEXT("/Script/Engine.Actor:GetDistanceTo"))
        .Get();

    return GetDistanceTo(this, OtherActor);
}

void AActor::ForceNetUpdate()
{
    static auto ForceNetUpdate = Anchor::VirtualFunction<void (*)(AActor*)>(TEXT("AActor::ForceNetUpdate"))
        .Exec(TEXT("/Script/Engine.Actor:ForceNetUpdate"))
        .Find("jmp [*+0x*]")
        .Get();

    ForceNetUpdate(this);
}

void AActor::FlushNetDormancy()
{
    static auto FlushNetDormancy = Anchor::Function<void (*)(AActor*)>(TEXT("AActor::FlushNetDormancy"))
        .Exec(TEXT("/Script/Engine.Actor:FlushNetDormancy"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    FlushNetDormancy(this);
}

bool AActor::Destroy(bool bNetForce, bool bShouldModifyLevel)
{
    static auto Destroy = Anchor::Function<bool (*)(AActor*, bool, bool)>(TEXT("AActor::Destroy"))
        .Exec(TEXT("/Script/Engine.Actor:K2_DestroyActor"))
        .Find("jmp [*+0x*]")
        .LogSlot(TEXT("AActor::K2_DestroyActor"))
        .Entry(TEXT("/Script/Engine.Default__Actor"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    return Destroy(this, bNetForce, bShouldModifyLevel);
}
