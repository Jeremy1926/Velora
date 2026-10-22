#include "pch.h"
#include "FortAthenaLivingWorldVolume.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "Athena/FortGameStateAthena.h"
#include "Components/SceneComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameplayTagContainer.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "WaterSubsystem.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortAthenaLivingWorldVolume::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume"));

    return Class;
}

void AFortAthenaLivingWorldVolume::OnCurrentPlaylistLoaded(FName PlaylistName, const FGameplayTagContainer& PlaylistContextTags)
{
    auto WaterSubsystem = UWaterSubsystem::GetWaterSubsystem(GetWorld());
    const float WaterLevelIndex = WaterSubsystem ? WaterSubsystem->GetOceanFloodHeight() / 384.0f : -1.0f;

    if (WaterLevelIndex != -1.0f)
    {
        const float WaterLevelIndexMin = GetWaterLevelIndexMin().GetValueAtLevel(0.0f);
        const float WaterLevelIndexMax = GetWaterLevelIndexMax().GetValueAtLevel(0.0f);

        if ((WaterLevelIndexMin != -1.0f && WaterLevelIndex < WaterLevelIndexMin) || (WaterLevelIndexMax != -1.0f && WaterLevelIndex > WaterLevelIndexMax))
            return;
    }

    auto EnvironmentQuery = GetEnvironmentQuery();
    auto RootComponent = GetRootComponent();

    if (!EnvironmentQuery || !RootComponent)
        return;

    FEnvQueryRequest QueryRequest(EnvironmentQuery, this);
    QueryRequest.SetFloatParam(FName(TEXT("SimpleGrid.GridSize")), RootComponent->GetBounds().SphereRadius);
    QueryRequest.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateUObject(this, &AFortAthenaLivingWorldVolume::OnEnvQueryFinished));
}

void AFortAthenaLivingWorldVolume::execOnCurrentPlaylistLoaded(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAthenaLivingWorldVolume, execOnCurrentPlaylistLoaded, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnCurrentPlaylistLoaded"))
        .Has("add [*+0x20], rax; mov rax, [*]; call [rax+0x*]; mov rcx, [rbp-0x*]")
        .Optional()
        .InExec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnCurrentPlaylistLoaded")));

    FName PlaylistName = NAME_None;
    FGameplayTagContainer PlaylistContextTagsTemp;

    Stack.StepCompiledIn(&PlaylistName);
    const FGameplayTagContainer& PlaylistContextTags = Stack.StepCompiledInRef<FGameplayTagContainer>(&PlaylistContextTagsTemp);
    Stack.Code += !!Stack.Code;

    OnCurrentPlaylistLoaded(PlaylistName, PlaylistContextTags);
}

void AFortAthenaLivingWorldVolume::OnEnvQueryFinished(TSharedPtr<FEnvQueryResult> Result)
{
    DETOUR(AFortAthenaLivingWorldVolume, OnEnvQueryFinished, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnCurrentPlaylistLoaded"))
        .Find("lea rax, [0x*]; mov [rsp+0x*], *; xor ecx, ecx")
        .Optional()
        .Target()
        .Inline());

    if (DETOUR_ORIGINAL(AFortAthenaLivingWorldVolume, OnEnvQueryFinished))
    {
        Original(MoveTemp(Result));
    }
    else if (Result.IsValid())
    {
        auto& Points = GetPoints();

        for (int32 Index = 0; Index < Result->Items.Num(); Index++)
            Points.Add(Result->GetItemAsLocation(Index));
    }

    if (GetPoints().Num() > 0 && GetWorld())
        UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(this);
}

bool AFortAthenaLivingWorldVolume::IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const
{
    DETOUR(AFortAthenaLivingWorldVolume, IsEnabled, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("cmp [rcx+0x*], rdi; jz 0x*; add rcx, 0x*; mov rax, [rcx]; call [rax+0x*]")
        .Optional()
        .Inline());

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto Volume = (const AFortAthenaLivingWorldVolume*)((const uint8*)this - PointerOffset);
    auto GameState = Volume->GetCachedGameState();
    auto& Points = Volume->GetPoints();

    if (!GameState || Points.Num() <= 0)
        return false;

    if (!PointFilter.bOnlyInSafeZone || GameState->GetSafeZoneState() == EFortSafeZoneState::None)
        return true;

    const FVector Location = Volume->GetActorLocation();
    const FVector SafeZoneCenter = GameState->GetSafeZoneCenter();
    float PointsRadiusSquared = 0.0f;

    for (auto& Point : Points)
        PointsRadiusSquared = FMath::Max(PointsRadiusSquared, (Location.X - Point.X) * (Location.X - Point.X) + (Location.Y - Point.Y) * (Location.Y - Point.Y));

    const float DeltaX = SafeZoneCenter.X - Location.X;
    const float DeltaY = SafeZoneCenter.Y - Location.Y;

    return GameState->GetSafeZoneRadius() >= FMath::Max(FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY) - FMath::Sqrt(PointsRadiusSquared), 0.0f);
}

bool AFortAthenaLivingWorldVolume::GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, FVector& OutPosition, FRotator& OutRotation) const
{
    DETOUR(AFortAthenaLivingWorldVolume, GetValidLocation, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0 && IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("add rcx, 0x*; mov [rsp+0x30], rbx; mov rax, [rcx]; call [rax+0x*]")
        .Optional()
        .Inline());

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto Volume = (const AFortAthenaLivingWorldVolume*)((const uint8*)this - PointerOffset);
    auto GameState = Volume->GetCachedGameState();
    auto& Points = Volume->GetPoints();
    const TArray<FVector>* ValidPoints = &Points;
    TArray<FVector> SafeZonePoints;

    if (Points.Num() <= 0)
        return false;

    if (PointFilter.bOnlyInSafeZone && GameState && GameState->GetSafeZoneState() != EFortSafeZoneState::None)
    {
        const FVector SafeZoneCenter = GameState->GetSafeZoneCenter();
        const float SafeZoneRadius = GameState->GetSafeZoneRadius();

        for (auto& Point : Points)
        {
            if ((SafeZoneCenter.X - Point.X) * (SafeZoneCenter.X - Point.X) + (SafeZoneCenter.Y - Point.Y) * (SafeZoneCenter.Y - Point.Y) <= SafeZoneRadius * SafeZoneRadius)
                SafeZonePoints.Add(Point);
        }

        ValidPoints = &SafeZonePoints;
    }

    if (ValidPoints->Num() <= 0)
        return false;

    OutPosition = (*ValidPoints)[FMath::Min((int32)(FMath::FRand() * ValidPoints->Num()), ValidPoints->Num() - 1)];
    OutRotation = Volume->GetActorRotation();

    return true;
}

AFortGameStateAthena* AFortAthenaLivingWorldVolume::GetCachedGameState() const
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    return *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>(this);
}

UEnvQuery* AFortAthenaLivingWorldVolume::GetEnvironmentQuery() const
{
    static auto EnvironmentQuery = GetClass()->FindPropertyByName(FName(TEXT("EnvironmentQuery")));

    return *EnvironmentQuery->ContainerPtrToValuePtr<UEnvQuery*>(this);
}

const FScalableFloat& AFortAthenaLivingWorldVolume::GetWaterLevelIndexMin() const
{
    static auto WaterLevelIndexMin = GetClass()->FindPropertyByName(FName(TEXT("WaterLevelIndexMin")));

    return *WaterLevelIndexMin->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& AFortAthenaLivingWorldVolume::GetWaterLevelIndexMax() const
{
    static auto WaterLevelIndexMax = GetClass()->FindPropertyByName(FName(TEXT("WaterLevelIndexMax")));

    return *WaterLevelIndexMax->ContainerPtrToValuePtr<FScalableFloat>(this);
}

TArray<FVector>& AFortAthenaLivingWorldVolume::GetPoints() const
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    return *(TArray<FVector>*)((uint8*)this + CachedGameState->GetOffset_ForInternal() + sizeof(AFortGameStateAthena*));
}
