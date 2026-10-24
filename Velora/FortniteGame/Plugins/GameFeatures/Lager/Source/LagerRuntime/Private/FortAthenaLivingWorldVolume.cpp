#include "pch.h"
#include "FortAthenaLivingWorldVolume.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "Athena/FortGameStateAthena.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
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
    EnablePointProvider();
}

void AFortAthenaLivingWorldVolume::EnablePointProvider()
{
    auto WaterSubsystem = UWaterSubsystem::GetWaterSubsystem(GetWorld());
    const float WaterLevelIndex = WaterSubsystem ? WaterSubsystem->GetOceanFloodHeight() / 384.0f : -1.0f;

    UE_LOG(LogVelora, Display, TEXT("diag lager volume %s enable water %f min %f max %f query %p root %p manager %p"), *GetNameSafe(this), WaterLevelIndex, GetWaterLevelIndexMin().GetValueAtLevel(0.0f), GetWaterLevelIndexMax().GetValueAtLevel(0.0f), GetEnvironmentQuery(), GetRootComponent(), GetCachedLivingWorldManager() ? *GetCachedLivingWorldManager() : nullptr);

    static auto DiagSceneClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.SceneComponent"));
    UE_LOG(LogVelora, Display, TEXT("diag lager vol %s cls %s scene %p init %d outer %s"), *GetNameSafe(this), *GetNameSafe(GetClass()), DiagSceneClass ? FindComponentByClass(DiagSceneClass) : nullptr, (int32)IsActorInitialized(), *GetNameSafe(GetOuter()));

    if (WaterLevelIndex != -1.0f)
    {
        const float WaterLevelIndexMin = GetWaterLevelIndexMin().GetValueAtLevel(0.0f);
        const float WaterLevelIndexMax = GetWaterLevelIndexMax().GetValueAtLevel(0.0f);

        if ((WaterLevelIndexMin != -1.0f && WaterLevelIndex < WaterLevelIndexMin) || (WaterLevelIndexMax != -1.0f && WaterLevelIndex > WaterLevelIndexMax))
            return;
    }

    if (auto CachedLivingWorldManager = GetCachedLivingWorldManager())
    {
        auto GameState = GetWorld() ? GetWorld()->GetGameState() : nullptr;

        GetCachedGameState() = Cast<AFortGameStateAthena>(GameState);
        *CachedLivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass()) : nullptr;

        if (auto bIsEnabled = GetbIsEnabled())
            *bIsEnabled = true;
    }

    auto EnvironmentQuery = GetEnvironmentQuery();
    auto RootComponent = GetRootComponent();

    if (!EnvironmentQuery || !RootComponent)
        return;

    FEnvQueryRequest QueryRequest(EnvironmentQuery, this);
    QueryRequest.SetFloatParam(FName(TEXT("SimpleGrid.GridSize")), RootComponent->GetBounds().SphereRadius);
    QueryRequest.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateUObject(this, &AFortAthenaLivingWorldVolume::OnEnvQueryFinished));
}

void AFortAthenaLivingWorldVolume::DisablePointProvider()
{
    if (auto bIsEnabled = GetbIsEnabled())
        *bIsEnabled = false;

    UFortAthenaLivingWorldManager::LivingWorldManagerUnregisterPointProvider(this);
}

void AFortAthenaLivingWorldVolume::execOnCurrentPlaylistLoaded(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAthenaLivingWorldVolume, execOnCurrentPlaylistLoaded, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnCurrentPlaylistLoaded"))
        .Has("add [*+0x20], rax; mov rax, [*]; call [rax+0x*]; mov rcx, [rbp-0x*]")
        .Or()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnCurrentPlaylistLoaded"))
        .Has("setnz al; add [*+0x20], rax; test rcx, rcx; jz 0x*; call 0x*")
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
        TArray<FVector> Points;
        auto PointProviderMaxRadiusSqr = GetPointProviderMaxRadiusSqr();
        auto CachedLivingWorldManager = GetCachedLivingWorldManager();
        const FVector Location = GetActorLocation();

        for (int32 Index = 0; Index < Result->Items.Num(); Index++)
        {
            const FVector Point = Result->GetItemAsLocation(Index);

            Points.Add(Point);

            if (PointProviderMaxRadiusSqr)
                *PointProviderMaxRadiusSqr = FMath::Max(*PointProviderMaxRadiusSqr, (Location.X - Point.X) * (Location.X - Point.X) + (Location.Y - Point.Y) * (Location.Y - Point.Y));
        }

        SetPoints(Points);

        if (CachedLivingWorldManager && *CachedLivingWorldManager && Points.Num() > 0 && GetWorld())
        {
            TArray<FVector> ClusteredEnvironmentQueryResults;

            (*CachedLivingWorldManager)->ClusterPoints(Points, ClusteredEnvironmentQueryResults);
            SetClusteredEnvironmentQueryResults(ClusteredEnvironmentQueryResults);
        }
    }

    auto bIsEnabled = GetbIsEnabled();

    UE_LOG(LogVelora, Display, TEXT("diag lager volume %s query finished valid %d points %d enabled %d"), *GetNameSafe(this), Result.IsValid() ? 1 : 0, GetPoints().Num(), bIsEnabled ? (int32)*bIsEnabled : -1);

    if (GetPoints().Num() > 0 && GetWorld() && (!bIsEnabled || *bIsEnabled))
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
        .Or()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("call 0x*; test al, al; jz 0x*; cmp [rbx+0x*], rsi; jz 0x*; lea rcx, [rbx-0x*]; call 0x*")
        .Optional()
        .Inline());

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto Volume = (const AFortAthenaLivingWorldVolume*)((const uint8*)this - PointerOffset);
    auto GameState = Volume->GetCachedGameState();
    const auto Points = Volume->GetPoints();
    auto SpawnLimiter = FFortAthenaLivingWorldPointProviderSpawnLimiter::Get(Volume);

    if (SpawnLimiter && !SpawnLimiter->CanSpawn(PointFilter.GetExpectedSpawnCount()))
        return false;

    if (!GameState || Points.Num() <= 0)
        return false;

    if (!PointFilter.GetbOnlyInSafeZone() || GameState->GetSafeZoneState() == EFortSafeZoneState::None)
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

bool AFortAthenaLivingWorldVolume::GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, TGameValue<FVector>& GameOutPosition, TGameValue<FRotator>& GameOutRotation) const
{
    DETOUR(AFortAthenaLivingWorldVolume, GetValidLocation, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0 && IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("add rcx, 0x*; mov [rsp+0x30], rbx; mov rax, [rcx]; call [rax+0x*]")
        .Or()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0 && IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("add rcx, 0x*; mov [rsp+0x30], rbx; call 0x*")
        .Optional()
        .Inline());

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto Volume = (const AFortAthenaLivingWorldVolume*)((const uint8*)this - PointerOffset);
    auto GameState = Volume->GetCachedGameState();
    const auto Points = Volume->GetPoints();
    const TArray<FVector>* ValidPoints = &Points;
    TArray<FVector> SafeZonePoints;

    if (Points.Num() <= 0)
        return false;

    if (PointFilter.GetbOnlyInSafeZone() && GameState && GameState->GetSafeZoneState() != EFortSafeZoneState::None)
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

    GameOutPosition = (*ValidPoints)[FMath::Min((int32)(FMath::FRand() * ValidPoints->Num()), ValidPoints->Num() - 1)];
    GameOutRotation = Volume->GetActorRotation();

    return true;
}

AFortGameStateAthena*& AFortAthenaLivingWorldVolume::GetCachedGameState() const
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    return *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>((void*)this);
}

UFortAthenaLivingWorldManager** AFortAthenaLivingWorldVolume::GetCachedLivingWorldManager()
{
    static auto CachedLivingWorldManager = GetClass()->FindPropertyByName(FName(TEXT("CachedLivingWorldManager")));

    return CachedLivingWorldManager ? CachedLivingWorldManager->ContainerPtrToValuePtr<UFortAthenaLivingWorldManager*>(this) : nullptr;
}

bool* AFortAthenaLivingWorldVolume::GetbIsEnabled()
{
    static auto bIsEnabled = Anchor::Offset(TEXT("AFortAthenaLivingWorldVolume::bIsEnabled"))
        .If(StaticClass()->FindPropertyByName(FName(TEXT("CachedLivingWorldManager"))))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:IsPointProviderEnabled"))
        .Optional()
        .Find("mov al, [rcx+0x*]")
        .Disp();

    return bIsEnabled > 0 ? (bool*)((uint8*)this + bIsEnabled) : nullptr;
}

void AFortAthenaLivingWorldVolume::SetClusteredEnvironmentQueryResults(const TArray<FVector>& InClusteredEnvironmentQueryResults)
{
    static auto InterfaceClusteredEnvironmentQueryResults = Anchor::Offset(TEXT("AFortAthenaLivingWorldVolume::ClusteredEnvironmentQueryResults"))
        .If(StaticClass()->FindPropertyByName(FName(TEXT("CachedLivingWorldManager"))))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Optional()
        .Find("movsxd rax, dword ptr [rbx+0x*]; mov rdi, [rbx+0x*]")
        .Find("mov rdi, [rbx+0x*]")
        .Disp();

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    if (InterfaceClusteredEnvironmentQueryResults <= 0)
        return;

    auto ClusteredEnvironmentQueryResults = (uint8*)this + PointerOffset + InterfaceClusteredEnvironmentQueryResults;

    if (!UE::LWC::IsEnabled())
    {
        *(TArray<FVector>*)ClusteredEnvironmentQueryResults = InClusteredEnvironmentQueryResults;
        return;
    }

    auto& GameClusteredEnvironmentQueryResults = *(TArray<FVector3d>*)ClusteredEnvironmentQueryResults;
    GameClusteredEnvironmentQueryResults.Empty();

    for (auto& ClusteredEnvironmentQueryResult : InClusteredEnvironmentQueryResults)
        GameClusteredEnvironmentQueryResults.Add(FVector3d(ClusteredEnvironmentQueryResult));
}

float* AFortAthenaLivingWorldVolume::GetPointProviderMaxRadiusSqr()
{
    static auto InterfacePointProviderMaxRadiusSqr = Anchor::Offset(TEXT("AFortAthenaLivingWorldVolume::PointProviderMaxRadiusSqr"))
        .If(StaticClass()->FindPropertyByName(FName(TEXT("CachedLivingWorldManager"))))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("sqrtss xmm0, [rbx+0x*]")
        .Or()
        .If(StaticClass()->FindPropertyByName(FName(TEXT("CachedLivingWorldManager"))))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Optional()
        .Find("sqrtss xmm1, [rbx+0x*]")
        .Disp();

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    return InterfacePointProviderMaxRadiusSqr > 0 ? (float*)((uint8*)this + PointerOffset + InterfacePointProviderMaxRadiusSqr) : nullptr;
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

int32 AFortAthenaLivingWorldVolume::GetPointsOffset() const
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    static auto InterfacePoints = Anchor::Offset(TEXT("AFortAthenaLivingWorldVolume::Points"))
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnActorSpawnedResult__DelegateSignature")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("cmp byte ptr [rbx+0x*], 0x0; jz 0x*; lea r1?, [rbx+0x*]")
        .Find("lea r1?, [rbx+0x*]")
        .Or()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnActorSpawnedResult__DelegateSignature")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaLivingWorldVolume"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Optional()
        .Find("cmp [rdi+0x*], bl; jz 0x*; lea r10, [rdi+0x*]")
        .Find("lea r10, [rdi+0x*]")
        .Disp();

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;
    static auto Points = InterfacePoints > 0 ? (int32)PointerOffset + InterfacePoints : CachedGameState->GetOffset_ForInternal() + (int32)sizeof(AFortGameStateAthena*);

    return Points;
}

TArray<FVector> AFortAthenaLivingWorldVolume::GetPoints() const
{
    auto GamePoints = (const uint8*)this + GetPointsOffset();

    if (!UE::LWC::IsEnabled())
        return *(const TArray<FVector>*)GamePoints;

    TArray<FVector> Points;

    for (auto& Point : *(const TArray<FVector3d>*)GamePoints)
        Points.Add((FVector)Point);

    return Points;
}

void AFortAthenaLivingWorldVolume::SetPoints(const TArray<FVector>& InPoints)
{
    auto GamePoints = (uint8*)this + GetPointsOffset();

    if (!UE::LWC::IsEnabled())
    {
        *(TArray<FVector>*)GamePoints = InPoints;
        return;
    }

    auto& Points = *(TArray<FVector3d>*)GamePoints;
    Points.Empty();

    for (auto& Point : InPoints)
        Points.Add(FVector3d(Point));
}
