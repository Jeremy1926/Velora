#include "pch.h"
#include "GameFramework/Actor.h"
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
    static auto SetLifeSpan = Scanner::Exec(TEXT("/Script/Engine.Actor:SetLifeSpan")).Find("call [*+0x*]").Slot(TEXT("AActor::SetLifeSpan"));

    Scanner::Virtual<void (*)(AActor*, float)>(this, SetLifeSpan)(this, InLifespan);
}

void AActor::K2_AttachToActor(AActor* ParentActor, FName SocketName, EAttachmentRule LocationRule, EAttachmentRule RotationRule, EAttachmentRule ScaleRule, bool bWeldSimulatedBodies)
{
    static auto K2_AttachToActor = Scanner::Function<void (*)(AActor*, AActor*, FName, EAttachmentRule, EAttachmentRule, EAttachmentRule, bool)>(Scanner::Native(TEXT("/Script/Engine.Actor:K2_AttachToActor")).Get(TEXT("AActor::K2_AttachToActor")));

    K2_AttachToActor(this, ParentActor, SocketName, LocationRule, RotationRule, ScaleRule, bWeldSimulatedBodies);
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

    return *NetDriverName->ContainerPtrToValuePtr<FName>(this);
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
    static auto Destroying = Scanner::String(TEXT("Destroying %s, which doesn't have a valid world pointer"));
    static auto bActorIsBeingDestroyed = Destroying.Find("test byte ptr [*], 0x*");
    static auto bActorIsBeingDestroyedSign = Destroying.Find("cmp byte ptr [*], 0x80");
    static auto Offset = bActorIsBeingDestroyed.Get() ? bActorIsBeingDestroyed.Disp(TEXT("AActor::bActorIsBeingDestroyed")) : bActorIsBeingDestroyedSign.Disp(TEXT("AActor::bActorIsBeingDestroyed"));
    static auto Mask = bActorIsBeingDestroyed.Get() ? bActorIsBeingDestroyed.Imm() : 0x80;

    return (*((uint8_t*)this + Offset) & Mask) || IsPendingKill();
}

bool AActor::IsActorInitialized() const
{
    static auto bActorInitialized = Scanner::String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. ")).Last("test byte ptr [*], 0x*");
    static auto Offset = bActorInitialized.Disp(TEXT("AActor::bActorInitialized"));
    static auto Mask = bActorInitialized.Imm();

    return *((uint8_t*)this + Offset) & Mask;
}

AActor* AActor::GetOwner() const
{
    static auto Owner = GetClass()->FindPropertyByName(FName(TEXT("Owner")));

    return *Owner->ContainerPtrToValuePtr<AActor*>(this);
}

UActorComponent* AActor::FindComponentByClass(const TSubclassOf<UActorComponent> ComponentClass) const
{
    static auto FindComponentByClass = Scanner::Native(TEXT("/Script/Engine.Actor:GetComponentByClass")).Find("jmp [rax+0x*]").Slot(TEXT("AActor::FindComponentByClass"));

    return Scanner::Virtual<UActorComponent* (*)(const AActor*, const TSubclassOf<UActorComponent>)>(this, FindComponentByClass)(this, ComponentClass);
}

ULevel* AActor::GetLevel() const
{
    return (ULevel*)GetOuter();
}

const AActor* AActor::GetNetOwner() const
{
    static auto GetNetOwner = [this]
    {
        auto Owner = GetClass()->FindPropertyByName(FName(TEXT("Owner")))->GetOffset_ForInternal();
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Getter = std::format("mov rax, [rcx+0x{:x}]; ret", Owner);

        return Scanner::Slot(TEXT("AActor::GetNetOwner"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Is(Getter); });
    }();

    return Scanner::Virtual<const AActor* (*)(const AActor*)>(this, GetNetOwner)(this);
}

bool AActor::IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const
{
    static auto IsRelevancyOwnerFor = []
    {
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));

        return Scanner::Slot(TEXT("AActor::IsRelevancyOwnerFor"), VTable, [](const Scanner::FCode& Entry) { return Entry.Is("cmp r8, rcx; setz al; ret"); });
    }();

    return Scanner::Virtual<bool (*)(const AActor*, const AActor*, const AActor*, const AActor*)>(this, IsRelevancyOwnerFor)(this, ReplicatedActor, ActorOwner, ConnectionActor);
}

void AActor::ForceNetUpdate()
{
    static auto ForceNetUpdate = Scanner::Exec(TEXT("/Script/Engine.Actor:ForceNetUpdate")).Find("jmp [*+0x*]").Slot(TEXT("AActor::ForceNetUpdate"));

    Scanner::Virtual<void (*)(AActor*)>(this, ForceNetUpdate)(this);
}

void AActor::FlushNetDormancy()
{
    static auto FlushNetDormancy = Scanner::Function<void (*)(AActor*)>(Scanner::Exec(TEXT("/Script/Engine.Actor:FlushNetDormancy")).Find("jmp 0x*").Target(TEXT("AActor::FlushNetDormancy")));

    FlushNetDormancy(this);
}

bool AActor::Destroy(bool bNetForce, bool bShouldModifyLevel)
{
    static auto Destroy = []
    {
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto K2_DestroyActor = Scanner::Exec(TEXT("/Script/Engine.Actor:K2_DestroyActor")).Find("jmp [*+0x*]").Slot(TEXT("AActor::K2_DestroyActor"));

        return Scanner::Function<bool (*)(AActor*, bool, bool)>(Scanner::At(Scanner::Virtual<const void*>(DefaultObject, K2_DestroyActor)).Find("jmp 0x*").Target(TEXT("AActor::Destroy")));
    }();

    return Destroy(this, bNetForce, bShouldModifyLevel);
}
