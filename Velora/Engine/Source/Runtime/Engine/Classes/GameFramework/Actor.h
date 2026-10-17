#pragma once

#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "Components/SceneComponent.h"
#include "Math/Vector.h"

class UActorChannel;
class ULevel;
class UNetDriver;

class AActor : public UObject
{
public:
    bool GetOnlyRelevantToOwner() const;

    bool GetTearOff() const;

    bool IsNetStartupActor() const;

    ENetRole GetRemoteRole() const;

    ENetDormancy GetNetDormancy() const;

    float GetNetUpdateFrequency() const;

    float GetMinNetUpdateFrequency() const;

    void SetMinNetUpdateFrequency(float InMinNetUpdateFrequency);

    int32 GetNetTag() const;

    void SetNetTag(int32 InNetTag);

    FName GetNetDriverName() const;

    USceneComponent* GetRootComponent() const;

    FORCEINLINE FVector GetActorLocation() const
    {
        auto RootComponent = GetRootComponent();

        return RootComponent ? RootComponent->GetComponentLocation() : FVector(0.f, 0.f, 0.f);
    }

    bool IsPendingKillPending() const;

    bool IsActorInitialized() const;

    ULevel* GetLevel() const;

    const AActor* GetNetOwner() const;

    bool IsRelevancyOwnerFor(const AActor* ReplicatedActor, const AActor* ActorOwner, const AActor* ConnectionActor) const;

    bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const;

    float GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth);

    bool GetNetDormancy(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth);

    void ForceNetUpdate();

    void CallPreReplication(UNetDriver* NetDriver);
};
