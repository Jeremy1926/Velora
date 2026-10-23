#pragma once

#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"
#include "UObject/SparseDelegate.h"

class APawn;
class UActorChannel;
class UActorComponent;
class ULevel;
class UNetDriver;

struct FActorDestroyedSignature : public FSparseDelegate
{
};

class AActor : public UObject
{
public:
    static UClass* StaticClass();

    bool GetOnlyRelevantToOwner() const;

    bool GetTearOff() const;

    bool IsNetStartupActor() const;

    ENetRole GetRemoteRole() const;

    bool HasAuthority() const;

    bool CanBeDamaged() const;

    FVector GetActorForwardVector() const;

    void SetLifeSpan(float InLifespan);

    bool TeleportTo(const FVector& DestLocation, const FRotator& DestRotation, bool bIsATest = false, bool bNoCheck = false);

    void K2_AttachToActor(AActor* ParentActor, FName SocketName, EAttachmentRule LocationRule, EAttachmentRule RotationRule, EAttachmentRule ScaleRule, bool bWeldSimulatedBodies);

    void K2_DetachFromActor(EDetachmentRule LocationRule, EDetachmentRule RotationRule, EDetachmentRule ScaleRule);

    ENetDormancy GetNetDormancy() const;

    float GetNetUpdateFrequency() const;

    float GetMinNetUpdateFrequency() const;

    void SetMinNetUpdateFrequency(float InMinNetUpdateFrequency);

    int32 GetNetTag() const;

    void SetNetTag(int32 InNetTag);

    FName GetNetDriverName() const;

    USceneComponent* GetRootComponent() const;

    APawn* GetInstigator() const;

    AActor* GetOwner() const;

    FActorDestroyedSignature& GetOnDestroyed();

    UActorComponent* FindComponentByClass(const TSubclassOf<UActorComponent> ComponentClass) const;

    template<class T>
    T* FindComponentByClass() const
    {
        return (T*)FindComponentByClass(T::StaticClass());
    }

    FORCEINLINE FVector GetActorLocation() const
    {
        auto RootComponent = GetRootComponent();

        return RootComponent ? RootComponent->GetComponentLocation() : FVector(0.f, 0.f, 0.f);
    }

    FORCEINLINE FRotator GetActorRotation() const
    {
        auto RootComponent = GetRootComponent();

        return RootComponent ? RootComponent->GetComponentRotation() : FRotator{ 0.f, 0.f, 0.f };
    }

    FORCEINLINE const FTransform& GetTransform() const
    {
        return ActorToWorld();
    }

    FORCEINLINE const FTransform& ActorToWorld() const
    {
        auto RootComponent = GetRootComponent();

        return RootComponent ? RootComponent->GetComponentTransform() : FTransform::Identity;
    }

    bool IsPendingKillPending() const;

    bool IsActorInitialized() const;

    ULevel* GetLevel() const;

    const AActor* GetNetOwner() const;

    bool IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const;

    bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const;

    float GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth);

    bool GetNetDormancy(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth);

    float GetDistanceTo(const AActor* OtherActor) const;

    void ForceNetUpdate();

    void FlushNetDormancy();

    bool Destroy(bool bNetForce = false, bool bShouldModifyLevel = true);

    void CallPreReplication(UNetDriver* NetDriver);
};
