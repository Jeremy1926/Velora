#include "pch.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* IFortAthenaLivingWorldPointProviderInterface::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"));

    return Class;
}

bool IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius()
{
    static auto GetValidLocation = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"));
    static auto bHasActorDensityComputationRadius = GetValidLocation && GetValidLocation->FindPropertyByName(FName(TEXT("ActorDensityComputationRadius")));

    return bHasActorDensityComputationRadius;
}

bool IFortAthenaLivingWorldPointProviderInterface::GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, FVector& OutPosition, FRotator& OutRotation) const
{
    if (HasActorDensityComputationRadius())
    {
        static auto GetValidLocationWithRadius = Anchor::VirtualFunction<bool (*)(const IFortAthenaLivingWorldPointProviderInterface*, const FFortAthenaLivingWorldPointProviderFilterRules&, float, FVector&, FRotator&)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::GetValidLocation"))
            .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
            .Find("call [rax+0x*]")
            .Get();

        return GetValidLocationWithRadius(this, PointFilter, ActorDensityComputationRadius, OutPosition, OutRotation);
    }

    static auto GetValidLocation = Anchor::VirtualFunction<bool (*)(const IFortAthenaLivingWorldPointProviderInterface*, const FFortAthenaLivingWorldPointProviderFilterRules&, FVector&, FRotator&)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::GetValidLocation"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Get();

    return GetValidLocation(this, PointFilter, OutPosition, OutRotation);
}

void IFortAthenaLivingWorldPointProviderInterface::GetFiltersTags(FGameplayTagContainer& FilterTags) const
{
    static auto GetFiltersTags = Anchor::VirtualFunction<void (*)(const IFortAthenaLivingWorldPointProviderInterface*, FGameplayTagContainer&)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::GetFiltersTags"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetFiltersTags"))
        .Find("call [rax+0x*]")
        .Get();

    GetFiltersTags(this, FilterTags);
}

void IFortAthenaLivingWorldPointProviderInterface::OnSpawnedActor(AActor* SpawnedActor) const
{
    static auto OnSpawnedActor = Anchor::VirtualFunction<void (*)(const IFortAthenaLivingWorldPointProviderInterface*, AActor*)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::OnSpawnedActor"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Get();

    OnSpawnedActor(this, SpawnedActor);
}

void IFortAthenaLivingWorldPointProviderInterface::OnSpawnedActorFailed()
{
    static auto OnSpawnedActorFailed = Anchor::VirtualFunction<void (*)(IFortAthenaLivingWorldPointProviderInterface*)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::OnSpawnedActorFailed"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActorFailed"))
        .Find("jmp [rax+0x*]")
        .Get();

    OnSpawnedActorFailed(this);
}

void IFortAthenaLivingWorldPointProviderInterface::OnEventRequested()
{
    static auto OnEventRequested = Anchor::VirtualFunction<void (*)(IFortAthenaLivingWorldPointProviderInterface*)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::OnEventRequested"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnEventRequested"))
        .Find("jmp [rax+0x*]")
        .Get();

    OnEventRequested(this);
}

bool IFortAthenaLivingWorldPointProviderInterface::IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const
{
    static auto IsEnabled = Anchor::VirtualFunction<bool (*)(const IFortAthenaLivingWorldPointProviderInterface*, const FFortAthenaLivingWorldPointProviderFilterRules&)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::IsEnabled"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Get();

    return IsEnabled(this, PointFilter);
}
