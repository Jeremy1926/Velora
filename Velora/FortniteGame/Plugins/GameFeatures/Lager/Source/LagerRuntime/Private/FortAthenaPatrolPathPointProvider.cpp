#include "pch.h"
#include "FortAthenaPatrolPathPointProvider.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "Athena/FortGameStateAthena.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortAthenaPatrolPathPointProvider::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider"));

    return Class;
}

bool AFortAthenaPatrolPathPointProvider::IsEnabled(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter) const
{
    DETOUR(AFortAthenaPatrolPathPointProvider, IsEnabled, Anchor::Hook()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Is("sub rsp, 0x28; cmp qword ptr [rcx+0x*], 0x0; jz 0x*; add rcx, 0x*; mov rax, [rcx]; call [rax+0x*]; xor al, al*")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:IsEnabled"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Is("push rbx; sub rsp, 0x20; mov edx, [rdx]; mov rbx, rcx; add rcx, 0x*; call 0x*; test al, al; jz 0x*; cmp qword ptr [rbx+0x*], 0x0; jz 0x*; lea rcx, [rbx-0x*]; call 0x*; xor al, al")
        .Optional()
        .Inline());

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto PointProvider = (const AFortAthenaPatrolPathPointProvider*)((const uint8*)this - PointerOffset);
    auto GameState = PointProvider->GetCachedGameState();
    auto PatrolPath = GameState ? PointProvider->GetRuntimePatrolPath() : nullptr;
    auto SpawnLimiter = FFortAthenaLivingWorldPointProviderSpawnLimiter::Get(PointProvider);

    if (SpawnLimiter && !SpawnLimiter->CanSpawn(PointFilter.GetExpectedSpawnCount()))
        return false;

    if (!PatrolPath)
        return false;

    static auto MaxConcurrentUsage = PatrolPath->GetClass()->FindPropertyByName(FName(TEXT("MaxConcurrentUsage")));
    static auto CurrentConcurrentUsage = PatrolPath->GetClass()->FindPropertyByName(FName(TEXT("CurrentConcurrentUsage")));
    const int32 MaxUsage = *MaxConcurrentUsage->ContainerPtrToValuePtr<int32>(PatrolPath);

    if (MaxUsage > -1 && *CurrentConcurrentUsage->ContainerPtrToValuePtr<int32>(PatrolPath) >= MaxUsage)
        return false;

    const FVector Location = PointProvider->GetActorLocation();

    if (PointFilter.GetbOnlyInSafeZone() && GameState->GetSafeZoneState() != EFortSafeZoneState::None)
    {
        const FVector SafeZoneCenter = GameState->GetSafeZoneCenter();
        const float DeltaX = SafeZoneCenter.X - Location.X;
        const float DeltaY = SafeZoneCenter.Y - Location.Y;

        if (FMath::Max(FMath::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY) - PointProvider->GetRadius(), 0.0f) > GameState->GetSafeZoneRadius())
            return false;
    }

    if (!PointFilter.GetbCheckForActorDensity())
        return true;

    auto LivingWorldManager = (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass());

    return LivingWorldManager && LivingWorldManager->GetActorDensityBudget(Location, PointProvider->GetRadius()) > 0;
}

bool AFortAthenaPatrolPathPointProvider::GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, FVector& OutPosition, FRotator& OutRotation) const
{
    DETOUR(AFortAthenaPatrolPathPointProvider, GetValidLocation, Anchor::Hook(static_cast<bool (AFortAthenaPatrolPathPointProvider::*)(const FFortAthenaLivingWorldPointProviderFilterRules&, FVector&, FRotator&) const>(&AFortAthenaPatrolPathPointProvider::GetValidLocation))
        .If(!IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius() && !FFortAthenaLivingWorldPointProviderFilterRules::HasDensityRules())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("cmp byte ptr [*+0x1], 0x0; jz 0x*; lea rcx, [*-0x*]; mov rax, [rcx]; call [rax+0x*]")
        .Optional()
        .Inline());
    DETOUR(AFortAthenaPatrolPathPointProvider, GetValidLocation, Anchor::Hook(static_cast<bool (AFortAthenaPatrolPathPointProvider::*)(const FFortAthenaLivingWorldPointProviderFilterRules&, FVector&, FRotator&) const>(&AFortAthenaPatrolPathPointProvider::GetValidLocation))
        .If(FFortAthenaLivingWorldPointProviderFilterRules::HasDensityRules() && !IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Optional()
        .InInterfaceVTables(StaticClass(), IFortAthenaLivingWorldPointProviderInterface::StaticClass(), "sub rsp, 0x28; cmp qword ptr [rcx+0x*], 0x0; jz 0x*; add rcx, 0x*; call 0x*; xor al, al*"));

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto PointProvider = (const AFortAthenaPatrolPathPointProvider*)((const uint8*)this - PointerOffset);
    auto GameState = PointProvider->GetCachedGameState();
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass()) : nullptr;

    if (FFortAthenaLivingWorldPointProviderFilterRules::HasDensityRules())
    {
        const AActor* SpawnPoint = PointProvider;

        if (!GameState)
            return false;

        if (PointProvider->GetbSpawnOnPatrolPath())
        {
            auto PatrolPath = PointProvider->GetRuntimePatrolPath();

            if (!PatrolPath)
                return false;

            static auto PatrolPoints = PatrolPath->GetClass()->FindPropertyByName(FName(TEXT("PatrolPoints")));
            static auto bUseRandomStartupPatrolPoint = PatrolPath->GetClass()->FindPropertyByName(FName(TEXT("bUseRandomStartupPatrolPoint")));
            auto& Points = *PatrolPoints->ContainerPtrToValuePtr<TArray<AActor*>>(PatrolPath);

            if (Points.Num() <= 0)
                return false;

            SpawnPoint = Points[*bUseRandomStartupPatrolPoint->ContainerPtrToValuePtr<bool>(PatrolPath) ? FMath::RandHelper(Points.Num()) : 0];

            if (!SpawnPoint)
                return false;
        }

        const FVector Location = SpawnPoint->GetActorLocation();

        if (PointFilter.GetbOnlyInSafeZone() && GameState->GetSafeZoneState() != EFortSafeZoneState::None)
        {
            const FVector SafeZoneCenter = GameState->GetSafeZoneCenter();
            const float SafeZoneRadius = GameState->GetSafeZoneRadius();

            if ((SafeZoneCenter.X - Location.X) * (SafeZoneCenter.X - Location.X) + (SafeZoneCenter.Y - Location.Y) * (SafeZoneCenter.Y - Location.Y) > SafeZoneRadius * SafeZoneRadius)
                return false;
        }

        if (PointFilter.GetbCheckForActorDensity() && (!LivingWorldManager || LivingWorldManager->GetActorDensityBudget(Location, 0.0f) <= 0))
            return false;

        OutPosition = Location;
        OutRotation = SpawnPoint->GetActorRotation();

        return true;
    }

    if (!PointFilter.GetbCheckForActorDensity())
        return Original(PointFilter, OutPosition, OutRotation);

    auto LocationFilter = PointFilter;
    LocationFilter.SetbCheckForActorDensity(false);

    if (!Original(LocationFilter, OutPosition, OutRotation))
        return false;

    return LivingWorldManager && LivingWorldManager->GetActorDensityBudget(OutPosition, 0.0f) > 0;
}

bool AFortAthenaPatrolPathPointProvider::GetValidLocation(const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, float ActorDensityComputationRadius, FVector& OutPosition, FRotator& OutRotation) const
{
    DETOUR(AFortAthenaPatrolPathPointProvider, GetValidLocation, Anchor::Hook(static_cast<bool (AFortAthenaPatrolPathPointProvider::*)(const FFortAthenaLivingWorldPointProviderFilterRules&, float, FVector&, FRotator&) const>(&AFortAthenaPatrolPathPointProvider::GetValidLocation))
        .If(IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("cmp byte ptr [*+0x1], 0x0; jz 0x*; lea rcx, [*-0x*]; mov rax, [rcx]; call [rax+0x*]")
        .Or()
        .If(IFortAthenaLivingWorldPointProviderInterface::HasActorDensityComputationRadius())
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:GetValidLocation"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("cmp byte ptr [*+0x5], 0x0; jz 0x*; lea rcx, [rbx-0x*]; call 0x*")
        .Optional()
        .Inline());

    if (!PointFilter.GetbCheckForActorDensity())
        return Original(PointFilter, ActorDensityComputationRadius, OutPosition, OutRotation);

    auto LocationFilter = PointFilter;
    LocationFilter.SetbCheckForActorDensity(false);

    if (!Original(LocationFilter, ActorDensityComputationRadius, OutPosition, OutRotation))
        return false;

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto PointProvider = (const AFortAthenaPatrolPathPointProvider*)((const uint8*)this - PointerOffset);
    auto GameState = PointProvider->GetCachedGameState();
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(UFortAthenaLivingWorldManager::StaticClass()) : nullptr;

    return LivingWorldManager && LivingWorldManager->GetActorDensityBudget(OutPosition, ActorDensityComputationRadius) > 0;
}

void AFortAthenaPatrolPathPointProvider::OnSpawnedActor(AActor* SpawnedActor)
{
    DETOUR(AFortAthenaPatrolPathPointProvider, OnSpawnedActor, Anchor::Hook()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Has("test byte ptr [*+0x*], 0x4; jz 0x*; test *, *; jz 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]")
        .Optional()
        .Inline());

    Original(SpawnedActor);

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    auto PointProvider = (AFortAthenaPatrolPathPointProvider*)((uint8*)this - PointerOffset);

    if (SpawnedActor && PointProvider->GetbDisableOnSuccessfulSpawn())
        PointProvider->DisablePointProvider();
}

void AFortAthenaPatrolPathPointProvider::OnCurrentPlaylistLoaded(FName PlaylistName, const FGameplayTagContainer& PlaylistContextTags)
{
    DETOUR(AFortAthenaPatrolPathPointProvider, OnCurrentPlaylistLoaded, Anchor::Hook()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:OnCurrentPlaylistLoaded"))
        .Is("sub rsp, 0x*; test byte ptr [rcx+0x*], 0x1*")
        .Optional()
        .Inline());

    Original(PlaylistName, PlaylistContextTags);

    if (GetbStartEnabled() && GetAssociatedPatrolPath())
        UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(this);
}

void AFortAthenaPatrolPathPointProvider::EnablePointProvider()
{
    DETOUR(AFortAthenaPatrolPathPointProvider, EnablePointProvider, Anchor::Hook()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:EnablePointProvider"))
        .Is("xor eax, eax; cmp [rdx+0x20], rax; setnz al; add rcx, 0x*; add [rdx+0x20], rax; mov rax, [rcx]; jmp [rax+0x*]")
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:EnablePointProvider"))
        .Optional()
        .Find("jmp *")
        .Is("jmp 0x*")
        .Target()
        .Inline());

    static auto bInterfaceEntry = Anchor::Exists()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:EnablePointProvider"))
        .Is("xor eax, eax; cmp [rdx+0x20], rax; setnz al; add rcx, 0x*; add [rdx+0x20], rax; mov rax, [rcx]; jmp [rax+0x*]")
        .Get();
    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = bInterfaceEntry ? (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject : 0;

    Original();

    auto PointProvider = (AFortAthenaPatrolPathPointProvider*)((uint8*)this - PointerOffset);

    if (PointProvider->GetAssociatedPatrolPath())
        UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(PointProvider);
}

void AFortAthenaPatrolPathPointProvider::InitPointProvider(AActor* PatrolPath, bool bUsePatrolPathTags)
{
    DETOUR(AFortAthenaPatrolPathPointProvider, InitPointProvider, Anchor::Hook()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:EnablePointProvider"))
        .Is("xor eax, eax; cmp [rdx+0x20], rax; setnz al; add rcx, 0x*; add [rdx+0x20], rax; mov rax, [rcx]; jmp [rax+0x*]")
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("xor r8d, r8d; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:EnablePointProvider"))
        .Is("xor eax, eax; cmp [rdx+0x20], rax; setnz al; add rcx, 0x*; add [rdx+0x20], rax; mov rax, [rcx]; jmp [rax+0x*]")
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("xor r8d, r8d; add rsp, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Optional()
        .Inline());

    Original(PatrolPath, bUsePatrolPathTags);

    if (bUsePatrolPathTags && PatrolPath)
        UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(this);
}

void AFortAthenaPatrolPathPointProvider::DisablePointProvider()
{
    DETOUR(AFortAthenaPatrolPathPointProvider, DisablePointProvider, Anchor::Hook()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("test byte ptr [*+0x*], 0x4; jz 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Is("sub rsp, 0x28; test rcx, rcx; jz 0x*; mov rax, [rcx]; call [rax+0x*]")
        .Optional()
        .Inline());

    UFortAthenaLivingWorldManager::LivingWorldManagerUnregisterPointProvider(this);
}

void AFortAthenaPatrolPathPointProvider::execDisablePointProvider(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAthenaPatrolPathPointProvider, execDisablePointProvider, Anchor::Hook()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:DisablePointProvider"))
        .Find("jmp 0x*")
        .Target()
        .Is("test rcx, rcx; jz 0x*; mov rax, [rcx]; jmp [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:DisablePointProvider"))
        .Is("sub rsp, 0x28; xor eax, eax; cmp [rdx+0x20], rax; setnz al; add [rdx+0x20], rax; test rcx, rcx; jz 0x*; mov rax, [rcx]; call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:DisablePointProvider"))
        .Is("sub rsp, 0x28; xor eax, eax; cmp [rdx+0x20], rax; setnz al; add [rdx+0x20], rax; test rcx, rcx; jz 0x*; call 0x*; add rsp, 0x28; ret")
        .Optional()
        .InExec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:DisablePointProvider")));

    Stack.Code += !!Stack.Code;

    DisablePointProvider();
}

AFortGameStateAthena* AFortAthenaPatrolPathPointProvider::GetCachedGameState() const
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    return *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>(this);
}

AActor* AFortAthenaPatrolPathPointProvider::GetRuntimePatrolPath() const
{
    static auto RuntimePatrolPathWeakPtr = GetClass()->FindPropertyByName(FName(TEXT("RuntimePatrolPathWeakPtr")));

    return RuntimePatrolPathWeakPtr ? RuntimePatrolPathWeakPtr->ContainerPtrToValuePtr<TWeakObjectPtr<AActor>>(this)->Get() : nullptr;
}

AActor* AFortAthenaPatrolPathPointProvider::GetAssociatedPatrolPath() const
{
    static auto AssociatedPatrolPath = GetClass()->FindPropertyByName(FName(TEXT("AssociatedPatrolPath")));

    return AssociatedPatrolPath ? *AssociatedPatrolPath->ContainerPtrToValuePtr<AActor*>(this) : nullptr;
}

bool AFortAthenaPatrolPathPointProvider::GetbStartEnabled() const
{
    static auto bStartEnabled = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bStartEnabled")));

    return bStartEnabled && bStartEnabled->GetPropertyValue_InContainer(this);
}

bool AFortAthenaPatrolPathPointProvider::GetbSpawnOnPatrolPath() const
{
    static auto bSpawnOnPatrolPath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bSpawnOnPatrolPath")));

    return bSpawnOnPatrolPath && bSpawnOnPatrolPath->GetPropertyValue_InContainer(this);
}

bool AFortAthenaPatrolPathPointProvider::GetbDisableOnSuccessfulSpawn() const
{
    static auto bDisableOnSuccessfulSpawn = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDisableOnSuccessfulSpawn")));

    return bDisableOnSuccessfulSpawn && bDisableOnSuccessfulSpawn->GetPropertyValue_InContainer(this);
}

float AFortAthenaPatrolPathPointProvider::GetRadius() const
{
    static auto Radius = Anchor::Offset(TEXT("AFortAthenaPatrolPathPointProvider::Radius"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnPatrolPathAdded"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov r8b, 0x1; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("sqrtss xmm0, *; movss [*+0x*], xmm0")
        .Find("movss [*+0x*], xmm0")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnPatrolPathAdded"))
        .Find("mov r8b, 0x1; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("sqrtss xmm0, *; movss [*+0x*], xmm0")
        .Find("movss [*+0x*], xmm0")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPatrolPathPointProvider:EnablePointProvider"))
        .Find("jmp 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("sqrtss xmm0, *; movss [*+0x*], xmm0")
        .Find("movss [*+0x*], xmm0")
        .Disp();

    return Radius > 0 ? *(const float*)((const uint8*)this + Radius) : 0.0f;
}
