#include "pch.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "FortAthenaLivingWorldEvent.h"
#include "FortAthenaLivingWorldEventData.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Scanner.h"

FFortAthenaLivingWorldPointProviderFilterRules::FFortAthenaLivingWorldPointProviderFilterRules(bool bOnlyInSafeZone, bool bCheckForActorDensity, int32 ExpectedSpawnCount)
    : Data{}
{
    if (HasEventRules())
        *(int32*)(Data + 0x10) = ExpectedSpawnCount;
    else if (GetFlagsOffset())
        *(int32*)Data = ExpectedSpawnCount;

    Data[HasEventRules() ? 0x29 : GetFlagsOffset()] = bOnlyInSafeZone;
    Data[HasEventRules() ? 0x2C : GetFlagsOffset() + 1] = bCheckForActorDensity;
}

FFortAthenaLivingWorldPointProviderFilterRules::FFortAthenaLivingWorldPointProviderFilterRules(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData, int32 ExpectedSpawnCount, float DensityComputationRange, float MaxActorDensity)
    : FFortAthenaLivingWorldPointProviderFilterRules(Event->GetbOnlyInSafeZone(), Event->GetbCheckForActorDensity(), ExpectedSpawnCount)
{
    if (!HasEventRules())
        return;

    auto TagDensityConditions = Event->GetTagDensityConditions();
    const int32 NumActorDescriptions = EventData ? FMath::Min(ExpectedSpawnCount, EventData->GetActorDescriptions().Num()) : 0;
    float DensityCostOfEvent = 0.0f;

    for (int32 Index = 0; Index < NumActorDescriptions; Index++)
    {
        auto ActorDensityValue = EventData->GetActorDescriptions()[Index].GetActorDensityValue();

        DensityCostOfEvent += ActorDensityValue ? ActorDensityValue->GetValueAtLevel(0.0f) : 0.0f;
    }

    *(void**)Data = TagDensityConditions ? TagDensityConditions->GetData() : nullptr;
    *(int32*)(Data + 0x8) = TagDensityConditions ? TagDensityConditions->Num() : 0;
    *(float*)(Data + 0x14) = DensityCostOfEvent;
    *(float*)(Data + 0x18) = DensityComputationRange;
    *(float*)(Data + 0x1C) = Event->GetMaxActorDensityOverride() < 0.0f ? MaxActorDensity : FMath::Min(Event->GetMaxActorDensityOverride(), MaxActorDensity);
    *(float*)(Data + 0x20) = Event->GetMinDistanceToPlayer();
    *(float*)(Data + 0x24) = Event->GetMaxDistanceToPlayer();
    Data[0x28] = Event->GetbCanSpawnWithoutMatchingPlayerDistance();
    Data[0x2A] = Event->GetbOnlyInNextSafeZone();
    Data[0x2B] = Event->GetbOnlyInNextSafeZoneIfStormShrinking();
}

UScriptStruct* FFortAthenaLivingWorldPointProviderFilterRules::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderFilterRules"));

    return Struct;
}

int32 FFortAthenaLivingWorldPointProviderFilterRules::GetFlagsOffset()
{
    static auto FlagsOffset = StaticStruct() && StaticStruct()->GetPropertiesSize() >= (int32)(sizeof(int32) * 2) ? (int32)sizeof(int32) : 0;

    return FlagsOffset;
}

bool FFortAthenaLivingWorldPointProviderFilterRules::HasEventRules()
{
    static auto bHasEventRules = StaticStruct() && StaticStruct()->GetPropertiesSize() >= (int32)sizeof(Data);

    return bHasEventRules;
}

int32 FFortAthenaLivingWorldPointProviderFilterRules::GetExpectedSpawnCount() const
{
    if (HasEventRules())
        return *(const int32*)(Data + 0x10);

    return GetFlagsOffset() ? *(const int32*)Data : 0;
}

bool FFortAthenaLivingWorldPointProviderFilterRules::GetbOnlyInSafeZone() const
{
    return Data[HasEventRules() ? 0x29 : GetFlagsOffset()] != 0;
}

bool FFortAthenaLivingWorldPointProviderFilterRules::GetbCheckForActorDensity() const
{
    return Data[HasEventRules() ? 0x2C : GetFlagsOffset() + 1] != 0;
}

void FFortAthenaLivingWorldPointProviderFilterRules::SetbCheckForActorDensity(bool bCheckForActorDensity)
{
    Data[HasEventRules() ? 0x2C : GetFlagsOffset() + 1] = bCheckForActorDensity;
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
