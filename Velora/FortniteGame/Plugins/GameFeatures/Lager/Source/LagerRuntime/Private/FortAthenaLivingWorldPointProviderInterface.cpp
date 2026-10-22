#include "pch.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Scanner.h"

FFortAthenaLivingWorldPointProviderFilterRules::FFortAthenaLivingWorldPointProviderFilterRules(bool bOnlyInSafeZone, bool bCheckForActorDensity, int32 ExpectedSpawnCount)
    : Data{}
{
    if (GetFlagsOffset())
        *(int32*)Data = ExpectedSpawnCount;

    Data[GetFlagsOffset()] = bOnlyInSafeZone;
    Data[GetFlagsOffset() + 1] = bCheckForActorDensity;
}

UScriptStruct* FFortAthenaLivingWorldPointProviderFilterRules::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderFilterRules"));

    return Struct;
}

int32 FFortAthenaLivingWorldPointProviderFilterRules::GetFlagsOffset()
{
    static auto FlagsOffset = StaticStruct() && StaticStruct()->GetPropertiesSize() >= (int32)sizeof(Data) ? (int32)sizeof(int32) : 0;

    return FlagsOffset;
}

int32 FFortAthenaLivingWorldPointProviderFilterRules::GetExpectedSpawnCount() const
{
    return GetFlagsOffset() ? *(const int32*)Data : 0;
}

bool FFortAthenaLivingWorldPointProviderFilterRules::GetbOnlyInSafeZone() const
{
    return Data[GetFlagsOffset()] != 0;
}

bool FFortAthenaLivingWorldPointProviderFilterRules::GetbCheckForActorDensity() const
{
    return Data[GetFlagsOffset() + 1] != 0;
}

void FFortAthenaLivingWorldPointProviderFilterRules::SetbCheckForActorDensity(bool bCheckForActorDensity)
{
    Data[GetFlagsOffset() + 1] = bCheckForActorDensity;
}

UScriptStruct* FFortAthenaLivingWorldPointProviderSpawnLimiter::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderSpawnLimiter"));

    return Struct;
}

FFortAthenaLivingWorldPointProviderSpawnLimiter* FFortAthenaLivingWorldPointProviderSpawnLimiter::Get(const AActor* PointProvider)
{
    auto SpawnLimiter = PointProvider && StaticStruct() ? PointProvider->GetClass()->FindPropertyByName(FName(TEXT("SpawnLimiter"))) : nullptr;

    return SpawnLimiter ? (FFortAthenaLivingWorldPointProviderSpawnLimiter*)SpawnLimiter->ContainerPtrToValuePtr<void>((void*)PointProvider) : nullptr;
}

bool FFortAthenaLivingWorldPointProviderSpawnLimiter::CanSpawn(int32 ExpectedSpawnCount)
{
    static auto MaxNumberOfSpawn = StaticStruct()->FindPropertyByName(FName(TEXT("MaxNumberOfSpawn")));
    const int32 MaxSpawnCount = (int32)MaxNumberOfSpawn->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f);

    return MaxSpawnCount < 0 || CurrentNumberOfSpawn + ExpectedSpawnCount <= MaxSpawnCount;
}

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

void IFortAthenaLivingWorldPointProviderInterface::OnSpawnedActor(AActor* SpawnedActor, const FVector& PositionFromProvider) const
{
    static auto OnSpawnedActor = Anchor::VirtualFunction<void (*)(const IFortAthenaLivingWorldPointProviderInterface*, AActor*, const FVector&)>(TEXT("IFortAthenaLivingWorldPointProviderInterface::OnSpawnedActor"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Get();

    OnSpawnedActor(this, SpawnedActor, PositionFromProvider);
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
