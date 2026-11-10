#include "pch.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldCategory.h"
#include "FortAthenaLivingWorldConfigData.h"
#include "FortAthenaLivingWorldDensityGrid.h"
#include "FortAthenaLivingWorldEncounter.h"
#include "FortAthenaLivingWorldEvent.h"
#include "FortAthenaLivingWorldEventData.h"
#include "FortAthenaLivingWorldPlayerTracker.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "FortAthenaLivingWorldVolume.h"
#include "FortAthenaPatrolPathPointProvider.h"
#include "Athena/AI/AthenaAISpawner.h"
#include "Athena/AI/AthenaAISystem.h"
#include "Athena/AI/FortAthenaAISpawnerData.h"
#include "Athena/FortAthenaActorSpawnerData.h"
#include "Athena/FortGameStateAthena.h"
#include "AI/FortAthenaAIBotController.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Pawns/FortPawn.h"
#include "Pawns/FortPlayerPawn.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Templates/Casts.h"
#include "Templates/Sorting.h"
#include "UObject/Class.h"
#include "UObject/SparseDelegate.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include <cfloat>

TArray<UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeDataEntry> UFortAthenaLivingWorldManager::EventRuntimeData;

TArray<UFortAthenaLivingWorldManager::FLivingWorldEventRequest> UFortAthenaLivingWorldManager::PendingEventRequests;

TArray<UFortAthenaLivingWorldManager::FLivingWorldPendingEQSSpawn> UFortAthenaLivingWorldManager::PendingEQSSpawns;

int32 UFortAthenaLivingWorldManager::NextEQSSpawnRequestID = INDEX_NONE;

TArray<UFortAthenaLivingWorldManager::FLivingWorldActorUpdateInfo> UFortAthenaLivingWorldManager::ActorToDespawnQueue;

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetTypeSize()
{
    return GetRespawnTimeOffset() + sizeof(float) + sizeof(int32);
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetRespawnTimeOffset()
{
    static auto RespawnTime = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventInstance::RespawnTime"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Optional()
        .Find("addss xmm*, [*+0x*]; movss [*+0x*], xmm*")
        .Find("movss [*+0x*], xmm*")
        .Disp();

    return RespawnTime;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActorsOffset()
{
    if (GetSpawnedActorPointProvidersOffset() <= 0)
        return GetRespawnTimeOffset() - (int32)sizeof(TArray<TWeakObjectPtr<AActor>>);

    static auto SpawnedActors = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventInstance::SpawnedActors"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov r9b, 0x1; lea rcx, [rdi+0x*]; mov edx, ebp; call 0x*")
        .Find("lea rcx, [rdi+0x*]")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov r9b, 0x1; mov edx, r15d; call 0x*")
        .Disp();

    return SpawnedActors;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActorPointProvidersOffset()
{
    static auto SpawnedActorPointProviders = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventInstance::SpawnedActorPointProviders"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActorDestroyed")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov r14d, ebp; lea r15, [rdi+0x*]")
        .Find("lea r15, [rdi+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActorDestroyed")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov r14d, r15d; lea r12, [rbx+0x*]")
        .Find("lea r12, [rbx+0x*]")
        .Optional()
        .Disp();

    return SpawnedActorPointProviders;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetEncounterInstanceOffset()
{
    static auto EncounterInstance = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventInstance::EncounterInstance"))
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() <= 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, [rbx+0x*]; test rcx, rcx; jz 0x*; mov rdx, [rcx+0x*]; test rdx, rdx; jz 0x*; mov rcx, r14; call 0x*")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, [rbx+0x*]; test rcx, rcx; jz 0x*; mov rdx, [rcx+0x*]; test rdx, rdx; jz 0x*; lea rcx, [rsi+0x*]; call 0x*")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rcx, [rbx+0x*]; test rcx, rcx; jz 0x*; mov rdx, [rcx+0x*]; test rdx, rdx; jz 0x*; lea rcx, [r14+0x*]; call 0x*")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov rax, [rbx+0x*]; test rax, rax; jz 0x*; mov rdx, [rax+0x*]; test rdx, rdx; jz 0x*; mov rcx, r12; call 0x*")
        .Optional()
        .Disp();

    return EncounterInstance;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetPendingSpawnRequestsOffset()
{
    static auto PendingSpawnRequests = GetSpawnedActorPointProvidersOffset() > 0 ? GetSpawnedActorPointProvidersOffset() + (int32)sizeof(TArray<FScriptInterface>) : (int32)(sizeof(FFortAthenaLivingWorldCategory*) + sizeof(FFortAthenaLivingWorldEvent*) + sizeof(TWeakObjectPtr<UFortAthenaLivingWorldEventData>));

    return PendingSpawnRequests;
}

UFortAthenaLivingWorldManager::FLivingWorldEventInstance::~FLivingWorldEventInstance()
{
    GetSpawnedActors().~TArray();

    if (GetPendingActorSpawnEventRequestsOffset() <= 0)
        GetPendingSpawnRequests().~TArray();

    if (auto SpawnedActorPointProviders = GetSpawnedActorPointProviders())
        SpawnedActorPointProviders->~TArray();
}

TArray<TWeakObjectPtr<AActor>>& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActors()
{
    return *(TArray<TWeakObjectPtr<AActor>>*)((uint8*)this + GetSpawnedActorsOffset());
}

TArray<FScriptInterface>* UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActorPointProviders()
{
    return GetSpawnedActorPointProvidersOffset() > 0 ? (TArray<FScriptInterface>*)((uint8*)this + GetSpawnedActorPointProvidersOffset()) : nullptr;
}

TArray<UFortAthenaLivingWorldManager::FLivingWorldPendingSpawnRequest>& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetPendingSpawnRequests()
{
    return *(TArray<FLivingWorldPendingSpawnRequest>*)((uint8*)this + GetPendingSpawnRequestsOffset());
}

UFortAthenaLivingWorldEncounterInstance*& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetEncounterInstance()
{
    return *(UFortAthenaLivingWorldEncounterInstance**)((uint8*)this + GetEncounterInstanceOffset());
}

float& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetRespawnTime()
{
    return *(float*)((uint8*)this + GetRespawnTimeOffset());
}

int32& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetActorCount()
{
    return *(int32*)((uint8*)this + GetRespawnTimeOffset() + sizeof(float));
}

UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::FLivingWorldEventRuntimeData()
{
    memset(Data, 0, sizeof(Data));

    GetActivationTime() = -1.0f;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::GetSpawnedCountOffset()
{
    static auto SpawnedCount = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::SpawnedCount"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Optional()
        .Find("mov r9d, r14d; mov r8, r12; mov rdx, rdi; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("sub ecx, [r14+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Optional()
        .Find("mov r9d, r14d; mov r8, r15; mov rdx, rsi; mov rcx, rdi; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("sub ecx, [r14+0x*]")
        .Disp();

    return SpawnedCount > 0 ? SpawnedCount : (int32)sizeof(float);
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::GetTotalPointProviderWeightOffset()
{
    static auto TotalPointProviderWeight = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::TotalPointProviderWeight"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("mov [rsp+0x20], rcx; lea r9, [*+0x*]; mov rcx, *; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("addss xmm0, [rsi+0x*]; add rbx, 0x*; movss [rsi+0x*], xmm0")
        .Optional()
        .Disp();

    return TotalPointProviderWeight > 0 ? TotalPointProviderWeight : (int32)(sizeof(float) + sizeof(int32));
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::GetDensityComputationRangeOffset()
{
    static auto DensityComputationRange = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::DensityComputationRange"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Optional()
        .Find("movss xmm2, dword ptr [r12+0x*]; lea rdx, [rbp-0x*]; mov rcx, rbx; call 0x*")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Optional()
        .Find("movss xmm2, dword ptr [r15+0x*]; lea rdx, [rbp-0x*]; mov rcx, rdi; call 0x*")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Optional()
        .Find("movss xmm2, dword ptr [r15+0x*]; lea rcx, [rdi+0x*]; lea rdx, [rbp-0x*]; call 0x*")
        .Disp();

    return DensityComputationRange > 0 ? DensityComputationRange : (int32)(sizeof(float) + sizeof(int32) + sizeof(float));
}

float& UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::GetActivationTime()
{
    return *(float*)Data;
}

int32& UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::GetSpawnedCount()
{
    return *(int32*)(Data + GetSpawnedCountOffset());
}

float& UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::GetTotalPointProviderWeight()
{
    return *(float*)(Data + GetTotalPointProviderWeightOffset());
}

float& UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::GetDensityComputationRange()
{
    return *(float*)(Data + GetDensityComputationRangeOffset());
}

UClass* UFortAthenaLivingWorldManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager"));

    return Class;
}

UFortAthenaLivingWorldManager::FLivingWorldPendingActorSpawnEventRequest::FRequestInfo::FRequestInfo()
{
    memset(Data, 0, sizeof(Data));
}

void UFortAthenaLivingWorldManager::FLivingWorldPendingActorSpawnEventRequest::FRequestInfo::SetLocation(const FVector& InLocation)
{
    TGameLayout<FVector>::Store(Data, InLocation);
}

FScriptInterface& UFortAthenaLivingWorldManager::FLivingWorldPendingActorSpawnEventRequest::FRequestInfo::GetPointProvider()
{
    return UE::LWC::IsEnabled() ? ((TLayout<FVector3d>*)Data)->PointProvider : ((TLayout<FVector>*)Data)->PointProvider;
}

int32& UFortAthenaLivingWorldManager::FLivingWorldPendingActorSpawnEventRequest::FRequestInfo::GetRequestID()
{
    return UE::LWC::IsEnabled() ? ((TLayout<FVector3d>*)Data)->RequestID : ((TLayout<FVector>*)Data)->RequestID;
}

int32& UFortAthenaLivingWorldManager::FLivingWorldPendingActorSpawnEventRequest::FRequestInfo::GetPreRegistrationHandle()
{
    return UE::LWC::IsEnabled() ? ((TLayout<FVector3d>*)Data)->PreRegistrationHandle : ((TLayout<FVector>*)Data)->PreRegistrationHandle;
}

int32 UFortAthenaLivingWorldManager::GetEventInstancesOffset()
{
    static auto bAnchorMissed = StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")) && (GetPendingActorSpawnEventRequestsOffset() <= 0 || FLivingWorldEventInstance::GetRespawnTimeOffset() <= 0 || FLivingWorldEventInstance::GetSpawnedActorsOffset() <= 0);

    if (bAnchorMissed)
        return 0;

    static auto EventInstances = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::EventInstances"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Optional()
        .Find("movzx eax, byte ptr [rax+0x*]; xor r14d, r14d; mov [rsp+0x*], eax; cmp [*+0x*], r14d")
        .Find("lea r12, [*+0x*]")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("xor r14d, r14d; test eax, eax; jle 0x*; lea *, [*+0x*]")
        .Find("lea *, [*+0x*]")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("xor r13d, r13d; test eax, eax; jle 0x*; lea *, [*+0x*]")
        .Find("lea *, [*+0x*]")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("test ecx, ecx; jle 0x*; lea r14, [rsi+0x*]")
        .Find("lea r14, [rsi+0x*]")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("test ecx, ecx; jle 0x*; lea rsi, [r14+0x*]")
        .Find("lea rsi, [r14+0x*]")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() <= 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("test ecx, ecx; jle 0x*; lea rsi, [rbp+0x*]")
        .Find("lea rsi, [rbp+0x*]")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("xor r14d, r14d; mov [rsp+0x*], eax; test ecx, ecx; jle 0x*; lea *, [*+0x*]")
        .Find("lea *, [*+0x*]")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea r14, [rsi+0x*]; mov [rsp+0x*], r12b; mov rcx, r14")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea r1*, [r*+0x*]; mov *[rbp-0x*], *; mov rcx, r1*; lea r9, [rbp-0x*]; mov r8, r*; lea rdx, [rbp-0x*]; call 0x*")
        .Disp();

    return EventInstances;
}

int32 UFortAthenaLivingWorldManager::GetPendingActorSpawnEventRequestsOffset()
{
    static auto PendingActorSpawnEventRequests = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::PendingActorSpawnEventRequests"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Optional()
        .Find("test ecx, ecx; jle 0x*; lea r12, [r13+0x*]")
        .Find("lea r12, [r13+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("test ecx, ecx; jle 0x*; lea r12, [r14+0x*]")
        .Find("lea r12, [r14+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("test ecx, ecx; jle 0x*; lea r12, [r15+0x*]")
        .Find("lea r12, [r15+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("test ecx, ecx; jle 0x*; lea rdi, [r13+0x*]")
        .Find("lea rdi, [r13+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("test ecx, ecx; jle 0x*; lea r15, [r13+0x*]")
        .Find("lea r15, [r13+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("lea rdi, [rcx+0x*]; movaps xmm0, *; mov rsi, rdx; mov r13, rcx")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("lea r13, [rcx+0x*]; movaps xmm0, *; mov rdi, rdx; mov r14, rcx")
        .Disp();

    return PendingActorSpawnEventRequests;
}

void UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(AActor* PointProvider)
{
    if (GetEventInstancesOffset() <= 0 && !HasEventRuntimes())
        return;

    auto World = PointProvider ? PointProvider->GetWorld() : nullptr;
    auto GameState = World ? World->GetGameState() : nullptr;
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(StaticClass()) : nullptr;

    if (!LivingWorldManager || !LivingWorldManager->IsLivingWorldEnabled() || !GameState->HasAuthority())
        return;

    if (HasEventRuntimes())
    {
        auto CachedLivingWorldManager = PointProvider->GetClass()->FindPropertyByName(FName(TEXT("CachedLivingWorldManager")));
        auto RegisteredPointProvidersCount = StaticClass()->FindPropertyByName(FName(TEXT("RegisteredPointProvidersCount")));

        if (CachedLivingWorldManager && *CachedLivingWorldManager->ContainerPtrToValuePtr<UFortAthenaLivingWorldManager*>(PointProvider) != LivingWorldManager)
        {
            *CachedLivingWorldManager->ContainerPtrToValuePtr<UFortAthenaLivingWorldManager*>(PointProvider) = LivingWorldManager;

            if (RegisteredPointProvidersCount)
                (*RegisteredPointProvidersCount->ContainerPtrToValuePtr<int32>(LivingWorldManager))++;
        }

        FScriptInterface PointProviderInterface;

        PointProviderInterface.SetObject(PointProvider);
        PointProviderInterface.SetInterface(PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()));

        LivingWorldManager->EnablePointProvider(PointProviderInterface);
        return;
    }

    LivingWorldManager->AddPointProvider(PointProvider);
}

void UFortAthenaLivingWorldManager::LivingWorldManagerUnregisterPointProvider(AActor* PointProvider)
{
    if (GetEventInstancesOffset() <= 0 && !HasEventRuntimes())
        return;

    auto World = PointProvider ? PointProvider->GetWorld() : nullptr;
    auto GameState = World ? World->GetGameState() : nullptr;
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(StaticClass()) : nullptr;

    if (!LivingWorldManager || !GameState->HasAuthority())
        return;

    if (HasEventRuntimes())
    {
        FScriptInterface PointProviderInterface;

        PointProviderInterface.SetObject(PointProvider);
        PointProviderInterface.SetInterface(PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()));

        LivingWorldManager->DisablePointProvider(PointProviderInterface);
        return;
    }

    LivingWorldManager->RemovePointProvider(PointProvider);
}

bool UFortAthenaLivingWorldManager::IsLivingWorldEnabled()
{
    return GetLagerEnabled().GetValueAtLevel(0.0f) > 0.0f && !FParse::Param(FCommandLine::Get(), TEXT("NoLager"));
}

void UFortAthenaLivingWorldManager::TickComponent(float DeltaTime)
{
    if (HasEventRuntimes())
    {
        TickEventRuntimes(DeltaTime);
        return;
    }

    auto World = GetWorld();
    auto& NextEventGenerationTime = GetNextEventGenerationTime();
    auto Config = GetConfig();

    if (!World || !Config || !Config->GetCategoryTable() || NextEventGenerationTime < 0.0f)
        return;

    auto PlayerTracker = GetPlayerTracker();

    if (PlayerTracker)
        PlayerTracker->UpdateTimeSinceLastRefresh(DeltaTime);

    if (World->GetTimeSeconds() >= NextEventGenerationTime)
    {
        NextEventGenerationTime = World->GetTimeSeconds() + Config->GetMinimumEventGenerationInterval().GetValueAtLevel(0.0f);

        if (PlayerTracker && GetCachedGameState())
            PlayerTracker->RefreshTree(World, GetCachedGameState());

        GenerateEvents();
    }

    UpdateEncounterInstances();
    ProcessEventRequests();
    ProcessEQSRequests();
    UpdateActorsInfo();
    DespawnActors();

    auto ActorDensityGrid = GetActorDensityGrid();
    auto MaxActorDensityUpdatesPerTick = Config->GetMaxActorDensityUpdatesPerTick();

    if (ActorDensityGrid && MaxActorDensityUpdatesPerTick)
        ActorDensityGrid->UpdateActors((int32)MaxActorDensityUpdatesPerTick->GetValueAtLevel(0.0f));
}

void UFortAthenaLivingWorldManager::GenerateEvents()
{
    auto Config = GetConfig();
    auto CategoryTable = Config && Config->GetCategoryTable() ? Config->GetCategoryTable()->LoadSynchronous() : nullptr;

    if (!CategoryTable || !GetCachedGameState())
        return;

    auto& EventInstances = GetEventInstances();
    const float TimeSeconds = GetWorld()->GetTimeSeconds();

    for (int32 Index = EventInstances.Num() - 1; Index >= 0; Index--)
    {
        if (!EventInstances[Index].GetSpawnedActors().Num() && (GetPendingActorSpawnEventRequestsOffset() > 0 || !EventInstances[Index].GetPendingSpawnRequests().Num()) && TimeSeconds >= EventInstances[Index].GetRespawnTime())
            EventInstances.RemoveAt(Index);
    }

    TArray<FFortAthenaLivingWorldCategory*> Categories;

    for (auto& CategoryPair : CategoryTable->RowMap)
        Categories.Add((FFortAthenaLivingWorldCategory*)CategoryPair.Value);

    if (FFortAthenaLivingWorldCategory::StaticStruct()->FindPropertyByName(FName(TEXT("Priority"))))
        Sort(Categories.GetData(), Categories.Num(), [](FFortAthenaLivingWorldCategory& A, FFortAthenaLivingWorldCategory& B) { return A.GetPriority() > B.GetPriority(); });

    for (auto Category : Categories)
    {
        if (!Category->GetbExcludeFromGlobalAIBudget() && (int32)Config->GetMaxActorCount().GetValueAtLevel(0.0f) - GetActorCount(nullptr, nullptr) <= 0)
            continue;

        if (!Category->IsRequiredByPlaylist(GetPlaylistContextTags()))
            continue;

        auto CategoryRuntimeData = FindOrCreateCategoryRuntimeData(Category);
        auto EventTable = CategoryRuntimeData ? CategoryRuntimeData->CurrentEventTable : Category->GetEventTable().LoadSynchronous();

        if (!EventTable)
            continue;

        GenerateEventRequests(nullptr, Category, EventTable);
    }
}

void UFortAthenaLivingWorldManager::GenerateEventRequests(UFortAthenaLivingWorldEncounterInstance* EncounterInstance, FFortAthenaLivingWorldCategory* Category, UDataTable* EventTable)
{
    auto Config = GetConfig();
    const bool bExcludeFromGlobalAIBudget = Category->GetbExcludeFromGlobalAIBudget();
    int32 ActorBudget = (int32)Config->GetMaxActorCount().GetValueAtLevel(0.0f) - GetActorCount(nullptr, nullptr);

    if (!bExcludeFromGlobalAIBudget && ActorBudget <= 0)
        return;

    int32 CategoryBudget = ComputeCategoryRemainingBudget(Category);
    TArray<FFortAthenaLivingWorldEvent*> ActiveEvents;
    float TotalWeight = 0.0f;

    if (EncounterInstance)
    {
        auto Encounter = EncounterInstance->GetEncounterDefinition().LoadSynchronous();
        auto& Stages = Encounter->GetStages();

        CategoryBudget = FMath::Min(CategoryBudget, (int32)Stages[EncounterInstance->GetCurrentStageIndex()].GetMaximumConcurrentNumberOfAI().GetValueAtLevel(0.0f) - GetTotalEncounterInstanceActorCount(EncounterInstance));
    }

    ComputeActiveEvents(EventTable, ActiveEvents, TotalWeight, EncounterInstance);

    for (int32 Attempt = CategoryBudget; Attempt > 0 && CategoryBudget > 0 && TotalWeight > 0.0f; Attempt--)
    {
        const float Pick = FMath::FRand() * TotalWeight;
        float Weight = 0.0f;
        FFortAthenaLivingWorldEvent* Event = nullptr;

        for (auto ActiveEvent : ActiveEvents)
        {
            if (ActiveEvent->GetWeight() <= 0.0f)
                continue;

            Weight += ActiveEvent->GetWeight();

            if (Pick <= Weight)
            {
                Event = ActiveEvent;
                break;
            }
        }

        if (!Event)
            break;

        auto EventData = Event->GetEventData().LoadSynchronous();
        const int32 EventBudget = FMath::Min(CategoryBudget, GetEventBudget(Event, EncounterInstance));
        const int32 NumActorDescriptions = EventData ? EventData->GetActorDescriptions().Num() : 0;
        auto MinActorCountToSpawn = EventData ? EventData->GetMinActorCountToSpawn() : nullptr;
        auto MaxActorCountToSpawn = EventData ? EventData->GetMaxActorCountToSpawn() : nullptr;
        const int32 MinActorCount = MinActorCountToSpawn ? (int32)MinActorCountToSpawn->GetValueAtLevel(0.0f) : NumActorDescriptions;
        const int32 MaxActorCount = MaxActorCountToSpawn ? (int32)MaxActorCountToSpawn->GetValueAtLevel(0.0f) : NumActorDescriptions;

        if (!NumActorDescriptions || MinActorCount > EventBudget)
        {
            ActiveEvents.RemoveSingle(Event);
            TotalWeight -= Event->GetWeight();
            continue;
        }

        const int32 ActorCount = FMath::Min(FMath::Min(FMath::RandRange(MinActorCount, MaxActorCount), NumActorDescriptions), EventBudget);

        if (ActorCount <= 0)
            continue;

        const FFortAthenaLivingWorldPointProviderFilterRules PointFilter(Event, EventData, ActorCount, FFortAthenaLivingWorldPointProviderFilterRules::HasDensityRules() ? FindOrCreateEventRuntimeData(Event, EncounterInstance).GetDensityComputationRange() : 0.0f, GetMaxActorDensity());
        auto PointProvider = GetRandomPointProvider(EventData->GetProviderFiltersTagQuery(), PointFilter, EncounterInstance, FindOrCreateEventRuntimeData(Event, EncounterInstance));

        if (!PointProvider)
        {
            ActiveEvents.RemoveSingle(Event);
            TotalWeight -= Event->GetWeight();
            continue;
        }

        auto PointProviderInterface = (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass());

        for (int32 Index = 0; Index < ActorCount; Index++)
            PointProviderInterface->OnEventRequested();

        FLivingWorldEventRequest EventRequest;
        EventRequest.LivingWorldManager = this;
        EventRequest.Category = Category;
        EventRequest.Event = Event;
        EventRequest.EventData = EventData;
        EventRequest.PointProvider = PointProvider;
        EventRequest.EncounterInstance = EncounterInstance;
        EventRequest.ActorCountToSpawn = ActorCount;

        PendingEventRequests.Add(EventRequest);
        FindOrCreateEventRuntimeData(Event, EncounterInstance).GetSpawnedCount() += ActorCount;

        if (auto CategoryRuntimeData = FindOrCreateCategoryRuntimeData(Category))
            CategoryRuntimeData->GameSpawnedCount += ActorCount;

        CategoryBudget -= ActorCount;
    }
}

void UFortAthenaLivingWorldManager::UpdateEncounterInstances()
{
    if (FLivingWorldEventInstance::GetEncounterInstanceOffset() <= 0)
        return;

    const float TimeSeconds = GetWorld()->GetTimeSeconds();

    for (auto EncounterInstance : GetRunningEncounterInstances())
    {
        if (!EncounterInstance || EncounterInstance->GetbIsPaused())
            continue;

        auto Encounter = EncounterInstance->GetEncounterDefinition().LoadSynchronous();

        if (!Encounter)
            continue;

        auto& LastGenerationTime = EncounterInstance->GetLastGenerationTime();

        if ((LastGenerationTime == 0.0f && !EncounterInstance->GetRegisteredPointProviders().Num()) || TimeSeconds <= LastGenerationTime + Encounter->GetIntervalBetweenEventGeneration().GetValueAtLevel(0.0f))
            continue;

        LastGenerationTime = TimeSeconds;

        auto Category = EncounterInstance->GetCategory();
        auto& Stages = Encounter->GetStages();
        const int32 CurrentStageIndex = EncounterInstance->GetCurrentStageIndex();

        if (!Category || CurrentStageIndex < 0 || CurrentStageIndex >= Stages.Num())
            continue;

        auto& Stage = Stages[CurrentStageIndex];
        const int32 EncounterBudget = FMath::Max(0, (int32)Stage.GetMaximumConcurrentNumberOfAI().GetValueAtLevel(0.0f) - GetTotalEncounterInstanceActorCount(EncounterInstance));
        auto EventTable = Stage.GetEventTable().LoadSynchronous();

        if (FMath::Min(EncounterBudget, ComputeCategoryRemainingBudget(Category)) > 0 && EventTable)
            GenerateEventRequests(EncounterInstance, Category, EventTable);
    }
}

int32 UFortAthenaLivingWorldManager::ComputeCategoryRemainingBudget(FFortAthenaLivingWorldCategory* Category)
{
    auto Config = GetConfig();
    const int32 GlobalBudget = FMath::Max(0, (int32)Config->GetMaxActorCount().GetValueAtLevel(0.0f) - GetActorCount(nullptr, nullptr));
    auto CategoryRuntimeData = FindOrCreateCategoryRuntimeData(Category);
    int32 CategoryBudget = FMath::Max(0, Category->GetMaxCount() - GetActorCount(Category, nullptr));

    if (CategoryRuntimeData && Category->GetMaxSpawnedCount() >= 0)
        CategoryBudget = FMath::Min(CategoryBudget, FMath::Max(0, Category->GetMaxSpawnedCount() - CategoryRuntimeData->GameSpawnedCount));

    return Category->GetbExcludeFromGlobalAIBudget() ? CategoryBudget : FMath::Min(GlobalBudget, CategoryBudget);
}

void UFortAthenaLivingWorldManager::ProcessEventRequests()
{
    auto Config = GetConfig();
    int32 EventSpawnBudget = Config ? (int32)Config->GetMaxEventSpawnPerTick().GetValueAtLevel(0.0f) : 0;

    for (int32 Index = 0; Index < PendingEventRequests.Num() && EventSpawnBudget > 0;)
    {
        auto LivingWorldManager = PendingEventRequests[Index].LivingWorldManager.Get();

        if (LivingWorldManager && LivingWorldManager != this)
        {
            Index++;
            continue;
        }

        const FLivingWorldEventRequest EventRequest = PendingEventRequests[Index];
        PendingEventRequests.RemoveAt(Index);

        if (!LivingWorldManager)
            continue;

        RequestSpawnToAISpawnerSystem(EventRequest);
        EventSpawnBudget--;
    }
}

void UFortAthenaLivingWorldManager::UpdateActorsInfo()
{
    auto Config = GetConfig();
    auto World = GetWorld();
    auto ActorInfoUpdateQueue = GetActorInfoUpdateQueue();

    if (!Config || !World || !ActorInfoUpdateQueue || ActorInfoUpdateQueue->IsEmpty())
        return;

    TArray<FLivingWorldActorUpdateInfo> ProcessedActorInfos;
    const int32 MaxActorUpdatesPerTick = (int32)Config->GetMaxActorUpdatesPerTick()->GetValueAtLevel(0.0f);
    const float TimeSeconds = World->GetTimeSeconds();
    int32 ActorUpdates = 0;

    while (!ActorInfoUpdateQueue->IsEmpty() && (MaxActorUpdatesPerTick <= -1 || ActorUpdates < MaxActorUpdatesPerTick))
    {
        FLivingWorldActorUpdateInfo ActorToUpdateInfo;
        ActorInfoUpdateQueue->Dequeue(ActorToUpdateInfo);

        auto Actor = ActorToUpdateInfo.Actor.Get();

        if (!Actor || Actor->IsPendingKill())
            continue;

        ActorUpdates++;

        if (auto ActorDensityGrid = GetActorDensityGrid())
            ActorDensityGrid->UpdateActor(Actor);

        for (const auto& TagDensityToUpdate : ActorToUpdateInfo.TagDensityToUpdate)
        {
            if (auto TagDensityGrid = GetTagDensityGrid(TagDensityToUpdate))
                TagDensityGrid->UpdateActor(Actor);
        }

        if (ActorToUpdateInfo.Event)
        {
            auto DespawnCondition = ActorToUpdateInfo.Event->GetDespawnCondition();

            if (IsActorCandidateForDespawn(Actor, DespawnCondition))
            {
                if (ActorToUpdateInfo.IsDespawnCandidateSinceTimestamp == -1.0f)
                    ActorToUpdateInfo.IsDespawnCandidateSinceTimestamp = TimeSeconds;

                if (ActorToUpdateInfo.IsDespawnCandidateSinceTimestamp + DespawnCondition->GetTimeAsCandidateBeforeDespawn() <= TimeSeconds)
                {
                    ActorToDespawnQueue.Add(ActorToUpdateInfo);
                    continue;
                }
            }
            else
            {
                ActorToUpdateInfo.IsDespawnCandidateSinceTimestamp = -1.0f;
            }
        }

        ProcessedActorInfos.Add(ActorToUpdateInfo);
    }

    for (auto& ProcessedActorInfo : ProcessedActorInfos)
        ActorInfoUpdateQueue->Enqueue(ProcessedActorInfo);
}

void UFortAthenaLivingWorldManager::DespawnActors()
{
    auto Config = GetConfig();
    auto MaxActorDespawnPerTick = Config ? Config->GetMaxActorDespawnPerTick() : nullptr;
    auto ActorInfoUpdateQueue = GetActorInfoUpdateQueue();

    if (!MaxActorDespawnPerTick || !ActorInfoUpdateQueue)
        return;

    const int32 DespawnCount = FMath::Min((int32)MaxActorDespawnPerTick->GetValueAtLevel(0.0f), ActorToDespawnQueue.Num());

    if (DespawnCount <= 0)
        return;

    for (int32 Index = 0; Index < DespawnCount; Index++)
    {
        auto& ActorToDespawnInfo = ActorToDespawnQueue[Index];
        auto Actor = ActorToDespawnInfo.Actor.Get();

        if (!Actor)
            continue;

        if (ActorToDespawnInfo.Event && !IsActorCandidateForDespawn(Actor, ActorToDespawnInfo.Event->GetDespawnCondition()))
        {
            ActorInfoUpdateQueue->Enqueue(ActorToDespawnInfo);
            continue;
        }

        if (auto Pawn = Cast<AFortPawn>(Actor))
            Pawn->Suicide();
        else
            Actor->Destroy();
    }

    ActorToDespawnQueue.RemoveAt(0, DespawnCount);
}

bool UFortAthenaLivingWorldManager::IsActorCandidateForDespawn(AActor* Actor, FFortAthenaLivingWorldEventDespawnCondition* DespawnCondition)
{
    if (!Actor || !DespawnCondition->GetbCanBeDespawned())
        return false;

    auto World = GetWorld();

    if (auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr)
    {
        const FVector SafeZoneCenter = GameState->GetSafeZoneCenter();
        const float SafeZoneRadius = GameState->GetSafeZoneRadius();
        const FVector ActorLocation = Actor->GetActorLocation();
        const float DeltaX = SafeZoneCenter.X - ActorLocation.X;
        const float DeltaY = SafeZoneCenter.Y - ActorLocation.Y;
        auto bCanDespawnInTheStorm = DespawnCondition->GetbCanDespawnInTheStorm();
        auto CanStartDespawningSafeZonePhase = DespawnCondition->GetCanStartDespawningSafeZonePhase();

        if (!bCanDespawnInTheStorm || !*bCanDespawnInTheStorm || DeltaX * DeltaX + DeltaY * DeltaY <= SafeZoneRadius * SafeZoneRadius)
        {
            if (GameState->GetGamePhaseStep() < DespawnCondition->GetCanStartDespawningPhaseStep())
                return false;

            if (CanStartDespawningSafeZonePhase && GameState->GetSafeZonePhase() < *CanStartDespawningSafeZonePhase)
                return false;
        }
    }

    const float MinDistanceToPlayer = DespawnCondition->GetMinDistanceToPlayer();

    if (MinDistanceToPlayer > 0.0f)
    {
        const float SquaredMinDistanceToPlayer = MinDistanceToPlayer * MinDistanceToPlayer;
        int32 NumberOfPlayerInRange = 0;
        TArray<FSimpleBVTreeElement> ScratchPad;

        GetPlayerTracker()->GetNumberOfPlayerPawnInRange(Actor->GetActorLocation(), MinDistanceToPlayer, TArrayView<const float>(&SquaredMinDistanceToPlayer, 1), TArrayView<int32>(&NumberOfPlayerInRange, 1), ScratchPad);

        if (NumberOfPlayerInRange)
            return false;
    }

    if (DespawnCondition->GetbCanDespawnWhenInCombatWithPlayer())
        return true;

    auto Pawn = Cast<AFortPawn>(Actor);
    auto Controller = Pawn ? Cast<AAIController>(Pawn->GetController()) : nullptr;
    auto PerceptionComponent = Controller ? Controller->GetPerceptionComponent() : nullptr;

    if (!PerceptionComponent)
        return true;

    TArray<AActor*> HostileActors;
    PerceptionComponent->GetPerceivedHostileActors(HostileActors);

    for (auto HostileActor : HostileActors)
    {
        auto HostilePlayerPawn = Cast<AFortPlayerPawn>(HostileActor);

        if (!HostilePlayerPawn)
            continue;

        auto HostileBotController = Cast<AFortAthenaAIBotController>(HostilePlayerPawn->GetController());

        if (!HostileBotController || HostileBotController->IsAnAthenaGameParticipant())
            return false;
    }

    return true;
}

void UFortAthenaLivingWorldManager::ProcessEQSRequests()
{
    for (int32 Index = PendingEQSSpawns.Num() - 1; Index >= 0; Index--)
    {
        if (!PendingEQSSpawns[Index].LivingWorldManager.Get())
            PendingEQSSpawns.RemoveAt(Index);
    }

    for (int32 Index = 0; Index < PendingEQSSpawns.Num(); Index++)
    {
        auto& PendingEQSSpawn = PendingEQSSpawns[Index];

        if (PendingEQSSpawn.LivingWorldManager.Get() != this)
            continue;

        if (PendingEQSSpawn.QueryID != INDEX_NONE && GetWorld()->GetTimeSeconds() < PendingEQSSpawn.QueryStartTime + 5.0f)
            return;

        auto EQSHelper = GetEQSHelper();
        auto RootComponent = EQSHelper ? EQSHelper->GetRootComponent() : nullptr;
        auto SpawnAroundEnvironmentQuery = (UEnvQuery*)PendingEQSSpawn.SpawnAroundEnvironmentQuery.Get();

        if (PendingEQSSpawn.QueryID == INDEX_NONE && RootComponent && SpawnAroundEnvironmentQuery)
        {
            RootComponent->K2_SetWorldLocationAndRotation(PendingEQSSpawn.DefaultPosition, PendingEQSSpawn.DefaultRotation, false, true);

            FEnvQueryRequest QueryRequest(SpawnAroundEnvironmentQuery, EQSHelper);
            PendingEQSSpawn.QueryStartTime = GetWorld()->GetTimeSeconds();
            PendingEQSSpawn.QueryID = QueryRequest.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateUObject(this, &UFortAthenaLivingWorldManager::OnEQSRequestFinished));

            if (PendingEQSSpawn.QueryID != INDEX_NONE)
                return;
        }

        const FLivingWorldPendingEQSSpawn FailedEQSSpawn = PendingEQSSpawn;
        PendingEQSSpawns.RemoveAt(Index);

        SpawnFromEQSRequest(FailedEQSSpawn, nullptr);
        return;
    }
}

void UFortAthenaLivingWorldManager::OnEQSRequestFinished(TSharedPtr<FEnvQueryResult> Result)
{
    for (int32 Index = 0; Index < PendingEQSSpawns.Num(); Index++)
    {
        if (!Result.IsValid() || PendingEQSSpawns[Index].LivingWorldManager.Get() != this || PendingEQSSpawns[Index].QueryID != Result->QueryID)
            continue;

        const FLivingWorldPendingEQSSpawn PendingEQSSpawn = PendingEQSSpawns[Index];
        PendingEQSSpawns.RemoveAt(Index);

        const int32 ItemCount = Result->Items.Num();
        const FVector Location = ItemCount > 0 ? Result->GetItemAsLocation(FMath::RandHelper(ItemCount)) : FVector(ForceInit);

        SpawnFromEQSRequest(PendingEQSSpawn, ItemCount > 0 ? &Location : nullptr);
        return;
    }
}

void UFortAthenaLivingWorldManager::SpawnFromEQSRequest(const FLivingWorldPendingEQSSpawn& PendingEQSSpawn, const FVector* Location)
{
    auto AISystem = Cast<UAthenaAISystem>(GetWorld()->GetAISystem());
    auto AISpawner = AISystem ? AISystem->GetAISpawner() : nullptr;
    auto SpawnerDataClass = PendingEQSSpawn.SpawnerDataClass.Get();
    auto ActorClass = PendingEQSSpawn.ActorClass.Get();
    FVector* PendingLocation = nullptr;
    FLivingWorldPendingActorSpawnEventRequest::FRequestInfo* PendingRequestInfo = nullptr;
    int32* PendingRequestID = nullptr;

    if (GetPendingActorSpawnEventRequestsOffset() > 0)
    {
        for (auto& PendingActorSpawnEventRequest : GetPendingActorSpawnEventRequests())
        {
            for (auto& RequestInfo : PendingActorSpawnEventRequest.RequestInfos)
            {
                if (RequestInfo.GetRequestID() == PendingEQSSpawn.RequestID)
                {
                    PendingRequestInfo = &RequestInfo;
                    PendingRequestID = &RequestInfo.GetRequestID();
                }
            }
        }
    }
    else
    {
        for (auto& EventInstance : GetEventInstances())
        {
            for (auto& PendingSpawnRequest : EventInstance.GetPendingSpawnRequests())
            {
                if (PendingSpawnRequest.RequestID == PendingEQSSpawn.RequestID)
                {
                    PendingLocation = &PendingSpawnRequest.Location;
                    PendingRequestID = &PendingSpawnRequest.RequestID;
                }
            }
        }
    }

    if (Location && PendingLocation)
        *PendingLocation = *Location;

    if (Location && PendingRequestInfo)
        PendingRequestInfo->SetLocation(*Location);

    if (Location && SpawnerDataClass && AISpawner)
    {
        const FTransform SpawnTransform(PendingEQSSpawn.DefaultRotation, *Location, FVector(1.0f, 1.0f, 1.0f));
        auto ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerDataClass, this);
        const int32 RequestID = ComponentList ? AISpawner->RequestSpawn(ComponentList, SpawnTransform, TDelegate<void(AActor*, int32)>::CreateUObject(this, &UFortAthenaLivingWorldManager::OnActorSpawned)) : INDEX_NONE;

        if (RequestID != INDEX_NONE && PendingRequestID)
        {
            *PendingRequestID = RequestID;
            return;
        }
    }

    if (Location && !SpawnerDataClass && ActorClass)
    {
        const FTransform SpawnTransform(PendingEQSSpawn.DefaultRotation, *Location, FVector(1.0f, 1.0f, 1.0f));

        OnActorSpawned(GetWorld()->SpawnActor(ActorClass, &SpawnTransform), PendingEQSSpawn.RequestID);
        return;
    }

    OnActorSpawned(nullptr, PendingEQSSpawn.RequestID);
}

void UFortAthenaLivingWorldManager::OnActorSpawned(AActor* SpawnedActor, int32 RequestID)
{
    static auto NativeOnActorSpawned = Anchor::Optional<void (*)(UFortAthenaLivingWorldManager*, AActor*, int32)>(TEXT("UFortAthenaLivingWorldManager::OnActorSpawned"))
        .If(GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Get();

    if (NativeOnActorSpawned)
        return NativeOnActorSpawned(this, SpawnedActor, RequestID);

    for (auto& EventInstance : GetEventInstances())
    {
        auto& PendingSpawnRequests = EventInstance.GetPendingSpawnRequests();

        for (int32 Index = 0; Index < PendingSpawnRequests.Num(); Index++)
        {
            if (PendingSpawnRequests[Index].RequestID != RequestID)
                continue;

            auto PointProvider = PendingSpawnRequests[Index].PointProvider.Get();
            const FVector PositionFromProvider = PendingSpawnRequests[Index].Location;
            const float DensityValue = PendingSpawnRequests[Index].DensityValue;
            const float DensityRange = PendingSpawnRequests[Index].DensityRange;
            PendingSpawnRequests.RemoveAt(Index);

            if (SpawnedActor)
            {
                OnActorSpawned(EventInstance, SpawnedActor, PointProvider, PositionFromProvider, DensityValue, DensityRange);
            }
            else if (auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr)
            {
                PointProviderInterface->OnSpawnedActorFailed();
            }

            return;
        }
    }
}

void UFortAthenaLivingWorldManager::OnActorSpawned(FLivingWorldEventInstance& EventInstance, AActor* SpawnedActor, AActor* PointProvider, const FVector& PositionFromProvider, float DensityValue, float DensityRange)
{
    auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr;

    EventInstance.GetSpawnedActors().Add(SpawnedActor);

    if (auto SpawnedActorPointProviders = EventInstance.GetSpawnedActorPointProviders())
    {
        FScriptInterface SpawnedActorPointProvider;
        SpawnedActorPointProvider.SetObject(PointProviderInterface ? PointProvider : nullptr);
        SpawnedActorPointProvider.SetInterface(PointProviderInterface);

        SpawnedActorPointProviders->Add(SpawnedActorPointProvider);
    }

    FScriptDelegate OnSpawnedActorDestroyDelegate;
    OnSpawnedActorDestroyDelegate.BindUFunction(this, FName(TEXT("OnSpawnedActorDestroy")));
    SpawnedActor->GetOnDestroyed().bIsBound |= FSparseDelegateStorage::Add(SpawnedActor, FName(TEXT("OnDestroyed")), OnSpawnedActorDestroyDelegate);

    if (auto SpawnedPawns = GetSpawnedPawns())
        SpawnedPawns->Add(SpawnedActor);

    auto EncounterInstance = FLivingWorldEventInstance::GetEncounterInstanceOffset() > 0 ? EventInstance.GetEncounterInstance() : nullptr;
    auto ActorDensityReservoir = EncounterInstance ? EncounterInstance->GetActorDensityReservoir() : nullptr;

    if (auto ActorDensityGrid = GetActorDensityGrid())
    {
        ActorDensityGrid->AddActor(SpawnedActor, DensityValue, DensityRange >= 0.0f ? DensityRange : FMath::Sqrt(SpawnedActor->GetNetCullDistanceSquared()));

        if (ActorDensityReservoir)
            ActorDensityGrid->UpdateActorDensityValue(ActorDensityReservoir, FMath::Max(0.0f, ActorDensityGrid->GetActorDensityValue(ActorDensityReservoir) - DensityValue));
    }

    if (auto ActorInfoUpdateQueue = GetActorInfoUpdateQueue())
    {
        FLivingWorldActorUpdateInfo ActorToUpdateInfo;
        ActorToUpdateInfo.Actor = SpawnedActor;
        ActorToUpdateInfo.Event = EventInstance.Event;

        ActorInfoUpdateQueue->Enqueue(ActorToUpdateInfo);
    }

    if (PointProviderInterface)
        PointProviderInterface->OnSpawnedActor(SpawnedActor, PositionFromProvider);

    if (EncounterInstance)
        EncounterInstance->GetOnActorSpawned().ProcessMulticastDelegate<UObject>(&SpawnedActor);
}

void UFortAthenaLivingWorldManager::ComputeActiveEvents(UDataTable* EventTable, TArray<FFortAthenaLivingWorldEvent*>& ActiveEvents, float& TotalWeight, UFortAthenaLivingWorldEncounterInstance* EncounterInstance)
{
    static auto IsDeactivated = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::IsDeactivated"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("mov [rsp+0x20], rcx; lea r9, [*+0x*]; mov rcx, *; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov byte ptr [*+0x*], 0x1")
        .Optional()
        .Disp();

    const float TimeSeconds = GetWorld()->GetTimeSeconds();

    for (auto& EventPair : EventTable->RowMap)
    {
        auto Event = (FFortAthenaLivingWorldEvent*)EventPair.Value;
        auto EventData = Event->GetEventData().LoadSynchronous();

        if (!EventData || !IsEventActive(Event, EventData))
            continue;

        auto& RuntimeData = FindOrCreateEventRuntimeData(Event, EncounterInstance);

        if (IsDeactivated > 0 && *(bool*)((uint8*)&RuntimeData + IsDeactivated))
            continue;

        if (RuntimeData.GetActivationTime() == -1.0f)
            RuntimeData.GetActivationTime() = FMath::FRand() * (Event->GetActivationMaxDelay() - Event->GetActivationMinDelay()) + Event->GetActivationMinDelay() + TimeSeconds;

        if (TimeSeconds < RuntimeData.GetActivationTime())
            continue;

        ActiveEvents.Add(Event);
        TotalWeight += Event->GetWeight();
    }
}

bool UFortAthenaLivingWorldManager::IsEventActive(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData)
{
    static auto NativeIsEventActive = Anchor::Optional<bool (*)(UFortAthenaLivingWorldManager*, FFortAthenaLivingWorldEvent*, UFortAthenaLivingWorldEventData*, bool, EAthenaGamePhaseStep, int32)>(TEXT("UFortAthenaLivingWorldManager::IsEventActive"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov [rsp+0x20], *l")
        .Find("call 0x*")
        .Target()
        .Get();

    auto GameState = GetCachedGameState();

    if (NativeIsEventActive)
        return NativeIsEventActive(this, Event, EventData, false, GameState->GetGamePhaseStep(), GameState->GetSafeZonePhase());

    return Event->IsActive(GetPlaylistContextTags()) && GameState->GetGamePhaseStep() >= Event->GetActivationGamePhaseStep() && GameState->GetSafeZonePhase() >= Event->GetActivationSafeZoneIndex();
}

int32 UFortAthenaLivingWorldManager::GetEventBudget(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEncounterInstance* EncounterInstance)
{
    const int32 SafeZonePhase = GetCachedGameState()->GetSafeZonePhase();
    auto& MaxCount = Event->GetMaxCount();
    const float MaxCountValue = MaxCount.Num() ? MaxCount[SafeZonePhase < MaxCount.Num() ? SafeZonePhase : MaxCount.Num() - 1] : 0.0f;
    const int32 Budget = FMath::Max(0, (int32)MaxCountValue - GetActorCount(nullptr, Event));
    auto& MaxSpawnedCount = Event->GetMaxSpawnedCount();

    if (MaxSpawnedCount.Num() && Event->GetMaxSpawnedCount(SafeZonePhase) == -1)
        return Budget;

    return FMath::Min(Budget, FMath::Max(0, Event->GetMaxSpawnedCount(SafeZonePhase) - FindOrCreateEventRuntimeData(Event, EncounterInstance).GetSpawnedCount()));
}

int32 UFortAthenaLivingWorldManager::GetTotalEncounterInstanceActorCount(UFortAthenaLivingWorldEncounterInstance* EncounterInstance)
{
    int32 ActorCount = 0;

    if (FLivingWorldEventInstance::GetEncounterInstanceOffset() <= 0)
        return ActorCount;

    for (auto& EventInstance : GetEventInstances())
    {
        if (EventInstance.GetEncounterInstance() == EncounterInstance)
            ActorCount += EventInstance.GetActorCount();
    }

    for (auto& EventRequest : PendingEventRequests)
    {
        if (EventRequest.LivingWorldManager.Get() == this && EventRequest.EncounterInstance.Get() == EncounterInstance)
            ActorCount += EventRequest.ActorCountToSpawn;
    }

    if (GetPendingActorSpawnEventRequestsOffset() > 0)
    {
        for (auto& PendingActorSpawnEventRequest : GetPendingActorSpawnEventRequests())
        {
            if (PendingActorSpawnEventRequest.RequestData.EncounterInstance == EncounterInstance)
                ActorCount += PendingActorSpawnEventRequest.RequestData.ActorCountToSpawn;
        }
    }

    return ActorCount;
}

int32 UFortAthenaLivingWorldManager::GetActorCount(const FFortAthenaLivingWorldCategory* Category, const FFortAthenaLivingWorldEvent* Event)
{
    int32 ActorCount = 0;

    for (auto& EventInstance : GetEventInstances())
    {
        if (!Category && !Event && EventInstance.Category && EventInstance.Category->GetbExcludeFromGlobalAIBudget())
            continue;

        if ((!Category || EventInstance.Category == Category) && (!Event || EventInstance.Event == Event))
            ActorCount += EventInstance.GetActorCount();
    }

    for (auto& EventRequest : PendingEventRequests)
    {
        auto EventData = EventRequest.EventData.Get();

        if (!Category && !Event && EventRequest.Category && EventRequest.Category->GetbExcludeFromGlobalAIBudget())
            continue;

        if (EventData && EventRequest.LivingWorldManager.Get() == this && (!Category || EventRequest.Category == Category) && (!Event || EventRequest.Event == Event))
            ActorCount += EventRequest.ActorCountToSpawn;
    }

    if (GetPendingActorSpawnEventRequestsOffset() > 0)
    {
        for (auto& PendingActorSpawnEventRequest : GetPendingActorSpawnEventRequests())
        {
            auto& RequestData = PendingActorSpawnEventRequest.RequestData;

            if (!Category && !Event && RequestData.Category && RequestData.Category->GetbExcludeFromGlobalAIBudget())
                continue;

            if ((!Category || RequestData.Category == Category) && (!Event || RequestData.Event == Event))
                ActorCount += RequestData.ActorCountToSpawn;
        }
    }

    return ActorCount;
}

AActor* UFortAthenaLivingWorldManager::GetRandomPointProvider(const FGameplayTagQuery& ProviderFiltersTagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, UFortAthenaLivingWorldEncounterInstance* EncounterInstance, FLivingWorldEventRuntimeData& RuntimeData)
{
    static auto ProviderFiltersEntries = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::ProviderFiltersEntries"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("mov [rsp+0x20], rcx; lea r9, [*+0x*]; mov rcx, *; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rsi+0x*]; call 0x*; imul rdi, rdi, 0x*")
        .Optional()
        .Disp();

    static auto MatchingPointProviders = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData::MatchingPointProviders"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("mov [rsp+0x20], rcx; lea r9, [*+0x*]; mov rcx, *; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea r9, [rsi+0x*]; mov al, [*+0x*]")
        .Optional()
        .Disp();

    TArray<AActor*> PointProviders;
    TArray<AActor*> ValidPointProviders;
    const bool bFromRuntimeData = ProviderFiltersEntries > 0 && MatchingPointProviders > 0;
    auto TagQuery = bFromRuntimeData ? nullptr : &ProviderFiltersTagQuery;

    if (bFromRuntimeData)
    {
        const float Pick = FMath::FRand() * RuntimeData.GetTotalPointProviderWeight();
        float Weight = 0.0f;

        for (auto& ProviderFiltersEntry : *(TRuntimeSizedArray<FPointProviderFilterEntry>*)((uint8*)&RuntimeData + ProviderFiltersEntries))
        {
            Weight += ProviderFiltersEntry.GetWeight().GetValueAtLevel(0.0f);

            if (Pick <= Weight)
            {
                TagQuery = &ProviderFiltersEntry.GetProviderFiltersTagQuery();
                break;
            }
        }

        for (auto& PointProvider : *(TArray<FScriptInterface>*)((uint8*)&RuntimeData + MatchingPointProviders))
            PointProviders.Add((AActor*)PointProvider.GetObject());
    }
    else if (EncounterInstance)
    {
        for (auto& PointProvider : EncounterInstance->GetRegisteredPointProviders())
            PointProviders.Add((AActor*)PointProvider.GetObject());
    }
    else if (HasPointProviderInterfaces())
    {
        for (auto& PointProvider : GetPointProviderInterfaces())
            PointProviders.Add((AActor*)PointProvider.GetObject());
    }
    else
    {
        PointProviders = GetPointProviders();
    }

    if (!EncounterInstance && !bFromRuntimeData)
    {
        for (auto RuntimePointProvider : GetRuntimePointProviderList())
            PointProviders.AddUnique(RuntimePointProvider);
    }

    for (auto PointProvider : PointProviders)
    {
        auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr;

        if (!PointProviderInterface)
            continue;

        FGameplayTagContainer FiltersTags;
        PointProviderInterface->GetFiltersTags(FiltersTags);

        if (TagQuery && !TagQuery->IsEmpty() && !TagQuery->Matches(FiltersTags))
            continue;

        if (PointProviderInterface->IsEnabled(PointFilter))
            ValidPointProviders.Add(PointProvider);
    }

    return ValidPointProviders.Num() ? ValidPointProviders[FMath::RandHelper(ValidPointProviders.Num())] : nullptr;
}

bool UFortAthenaLivingWorldManager::RequestSpawnToAISpawnerSystem(const FLivingWorldEventRequest& EventRequest)
{
    auto AISystem = Cast<UAthenaAISystem>(GetWorld()->GetAISystem());
    auto AISpawner = AISystem ? AISystem->GetAISpawner() : nullptr;
    auto Event = EventRequest.Event;
    auto EventData = EventRequest.EventData.Get();
    auto PointProvider = EventRequest.PointProvider.Get();
    auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr;
    const int32 ActorCount = EventData ? FMath::Min(EventRequest.ActorCountToSpawn, EventData->GetActorDescriptions().Num()) : 0;
    auto EncounterInstance = EventRequest.EncounterInstance.Get();
    const FFortAthenaLivingWorldPointProviderFilterRules PointFilter(Event, EventData, ActorCount, FFortAthenaLivingWorldPointProviderFilterRules::HasDensityRules() ? FindOrCreateEventRuntimeData(Event, EncounterInstance).GetDensityComputationRange() : 0.0f, GetMaxActorDensity());
    const auto SpawnedCallback = TDelegate<void(AActor*, int32)>::CreateUObject(this, &UFortAthenaLivingWorldManager::OnActorSpawned);
    auto Config = GetConfig();
    auto DensityComputationDistance = Config ? Config->GetDensityComputationDistance() : nullptr;
    const float ActorDensityComputationRadius = DensityComputationDistance ? DensityComputationDistance->GetValueAtLevel(0.0f) : 0.0f;
    TArray<FLivingWorldPendingSpawnRequest> PendingSpawnRequests;
    TArray<AActor*> SpawnedActors;
    TArray<FLivingWorldPendingSpawnRequest> SpawnedActorRequests;
    bool bHasDefaultPosition = false;
    FVector DefaultPosition(ForceInit);
    FRotator DefaultRotation{ 0.0f, 0.0f, 0.0f };

    for (int32 Index = 0; PointProviderInterface && Index < ActorCount; Index++)
    {
        auto& ActorDescription = EventData->GetActorDescriptions()[Index];
        auto SpawnerDataClass = GetSpawnerDataClass(ActorDescription);
        auto ActorSpawnerData = SpawnerDataClass ? Cast<UFortAthenaActorSpawnerData>(SpawnerDataClass->GetDefaultObject()) : nullptr;
        auto ActorClass = ActorSpawnerData ? ActorSpawnerData->GetActorClass() : SpawnerDataClass ? nullptr : ActorDescription.GetActorClass().LoadSynchronous();

        auto ActorDensityValue = ActorDescription.GetActorDensityValue();
        auto DensityComputationRangeOverride = ActorDescription.GetDensityComputationRangeOverride();
        FLivingWorldPendingSpawnRequest PendingSpawnRequest;

        PendingSpawnRequest.PointProvider = PointProvider;
        PendingSpawnRequest.DensityValue = ActorDensityValue ? ActorDensityValue->GetValueAtLevel(0.0f) : 0.0f;
        PendingSpawnRequest.DensityRange = DensityComputationRangeOverride ? DensityComputationRangeOverride->GetValueAtLevel(0.0f) : -1.0f;

        if (ActorSpawnerData)
            SpawnerDataClass = nullptr;
        FVector Location(ForceInit);
        FRotator Rotation{ 0.0f, 0.0f, 0.0f };

        if (SpawnerDataClass ? !AISpawner : !ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
        {
            PointProviderInterface->OnSpawnedActorFailed();
            continue;
        }

        if (ActorDescription.GetbSpawnAroundDefaultPoint() && bHasDefaultPosition)
        {
            Location = DefaultPosition;
            Rotation = DefaultRotation;
        }
        else if (PointProviderInterface->GetValidLocation(PointFilter, ActorDensityComputationRadius, Location, Rotation))
        {
            if (!bHasDefaultPosition)
            {
                DefaultPosition = Location;
                DefaultRotation = Rotation;
                bHasDefaultPosition = true;
            }
        }
        else
        {
            PointProviderInterface->OnSpawnedActorFailed();
            continue;
        }

        auto SpawnAroundEnvironmentQuery = ActorDescription.GetbSpawnAroundDefaultPoint() ? ActorDescription.GetSpawnAroundEnvironmentQuery() : nullptr;

        if (SpawnAroundEnvironmentQuery && GetEQSHelper())
        {
            FLivingWorldPendingEQSSpawn PendingEQSSpawn;
            PendingEQSSpawn.LivingWorldManager = this;
            PendingEQSSpawn.PointProvider = PointProvider;
            PendingEQSSpawn.SpawnerDataClass = SpawnerDataClass;
            PendingEQSSpawn.ActorClass = ActorClass;
            PendingEQSSpawn.SpawnAroundEnvironmentQuery = (UObject*)SpawnAroundEnvironmentQuery;
            PendingEQSSpawn.DefaultPosition = DefaultPosition;
            PendingEQSSpawn.DefaultRotation = DefaultRotation;
            PendingEQSSpawn.RequestID = --NextEQSSpawnRequestID;

            PendingEQSSpawns.Add(PendingEQSSpawn);

            PendingSpawnRequest.RequestID = PendingEQSSpawn.RequestID;
            PendingSpawnRequest.Location = DefaultPosition;

            PendingSpawnRequests.Add(PendingSpawnRequest);
            continue;
        }

        const FTransform SpawnTransform(Rotation, Location, FVector(1.0f, 1.0f, 1.0f));

        if (!SpawnerDataClass)
        {
            auto SpawnedActor = GetWorld()->SpawnActor(ActorClass, &SpawnTransform);

            if (!SpawnedActor)
            {
                PointProviderInterface->OnSpawnedActorFailed();
                continue;
            }

            PendingSpawnRequest.Location = Location;

            SpawnedActors.Add(SpawnedActor);
            SpawnedActorRequests.Add(PendingSpawnRequest);
            continue;
        }

        auto ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerDataClass, this);
        const int32 RequestID = ComponentList ? AISpawner->RequestSpawn(ComponentList, SpawnTransform, SpawnedCallback) : INDEX_NONE;

        if (RequestID == INDEX_NONE)
        {
            PointProviderInterface->OnSpawnedActorFailed();
            continue;
        }

        PendingSpawnRequest.RequestID = RequestID;
        PendingSpawnRequest.Location = Location;

        PendingSpawnRequests.Add(PendingSpawnRequest);
    }

    const int32 SpawnedCount = PendingSpawnRequests.Num() + SpawnedActors.Num();

    FindOrCreateEventRuntimeData(Event, EncounterInstance).GetSpawnedCount() -= ActorCount - SpawnedCount;

    if (auto CategoryRuntimeData = FindOrCreateCategoryRuntimeData(EventRequest.Category))
        CategoryRuntimeData->GameSpawnedCount -= ActorCount - SpawnedCount;

    if (!SpawnedCount)
        return false;

    if (GetPendingActorSpawnEventRequestsOffset() > 0)
    {
        auto& PendingActorSpawnEventRequest = *new(GetPendingActorSpawnEventRequests()) FLivingWorldPendingActorSpawnEventRequest();
        TArray<int32> SpawnedActorRequestIDs;

        PendingActorSpawnEventRequest.RequestData.Category = EventRequest.Category;
        PendingActorSpawnEventRequest.RequestData.Event = Event;
        PendingActorSpawnEventRequest.RequestData.EventData = EventData;
        PendingActorSpawnEventRequest.RequestData.EncounterInstance = EncounterInstance;
        PendingActorSpawnEventRequest.RequestData.ActorCountToSpawn = SpawnedCount;

        for (int32 Index = 0; Index < SpawnedCount; Index++)
        {
            auto& RequestInfo = *new(PendingActorSpawnEventRequest.RequestInfos) FLivingWorldPendingActorSpawnEventRequest::FRequestInfo();

            RequestInfo.SetLocation(Index < PendingSpawnRequests.Num() ? PendingSpawnRequests[Index].Location : SpawnedActorRequests[Index - PendingSpawnRequests.Num()].Location);
            RequestInfo.GetRequestID() = Index < PendingSpawnRequests.Num() ? PendingSpawnRequests[Index].RequestID : SpawnedActorRequestIDs[SpawnedActorRequestIDs.Add(--NextEQSSpawnRequestID)];
            RequestInfo.GetPreRegistrationHandle() = INDEX_NONE;
            RequestInfo.GetPointProvider().SetObject(PointProvider);
            RequestInfo.GetPointProvider().SetInterface(PointProviderInterface);
        }

        for (int32 Index = 0; Index < SpawnedActors.Num(); Index++)
            OnActorSpawned(SpawnedActors[Index], SpawnedActorRequestIDs[Index]);

        return true;
    }

    auto& EventInstances = GetEventInstances();
    auto& EventInstance = EventInstances[EventInstances.AddUninitialized()];

    memset(&EventInstance, 0, FLivingWorldEventInstance::GetTypeSize());

    EventInstance.Category = EventRequest.Category;
    EventInstance.Event = Event;
    EventInstance.EventData = EventData;
    EventInstance.GetPendingSpawnRequests() = PendingSpawnRequests;
    EventInstance.GetActorCount() = SpawnedCount;

    if (FLivingWorldEventInstance::GetEncounterInstanceOffset() > 0)
        EventInstance.GetEncounterInstance() = EncounterInstance;

    for (int32 Index = 0; Index < SpawnedActors.Num(); Index++)
        OnActorSpawned(EventInstance, SpawnedActors[Index], PointProvider, SpawnedActorRequests[Index].Location, SpawnedActorRequests[Index].DensityValue, SpawnedActorRequests[Index].DensityRange);

    return true;
}

UClass* UFortAthenaLivingWorldManager::GetSpawnerDataClass(FFortAthenaLivingWorldEventDataActorSpawnDescription& ActorDescription)
{
    if (auto SpawnerData = ActorDescription.GetSpawnerData().LoadSynchronous())
        return SpawnerData;

    TArray<UClass*> SpawnerDataClasses;

    for (auto LoadedSpawnerDataClass : GetLoadedSpawnerDataClass())
    {
        auto DefaultObject = LoadedSpawnerDataClass ? LoadedSpawnerDataClass->GetDefaultObject() : nullptr;
        auto AISpawnerData = Cast<UFortAthenaAISpawnerData>(DefaultObject);
        auto ActorSpawnerData = Cast<UFortAthenaActorSpawnerData>(DefaultObject);
        auto DescriptorTag = AISpawnerData ? &AISpawnerData->GetDescriptorTag() : ActorSpawnerData ? &ActorSpawnerData->GetDescriptorTag() : nullptr;

        if (DescriptorTag && ActorDescription.GetSpawnerDataTagQuery().Matches(*DescriptorTag))
            SpawnerDataClasses.Add(LoadedSpawnerDataClass);
    }

    return SpawnerDataClasses.Num() ? SpawnerDataClasses[FMath::RandHelper(SpawnerDataClasses.Num())] : nullptr;
}

UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData& UFortAthenaLivingWorldManager::FindOrCreateEventRuntimeData(const FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEncounterInstance* EncounterInstance)
{
    static auto NativeFindOrCreateEventRuntimeData = Anchor::Optional<FLivingWorldEventRuntimeData* (*)(UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*)>(TEXT("UFortAthenaLivingWorldManager::FindOrCreateEventRuntimeData"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("mov rcx, *; call 0x*; mov rbx, [rsp+0x*]")
        .Find("call 0x*")
        .Target()
        .Find("mov rdx, *; mov rcx, *; mov *, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rdx, *; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto NativeFindOrAddEventRuntimeData = Anchor::Optional<FLivingWorldEventRuntimeData* (*)(const FFortAthenaLivingWorldEvent*, void*)>(TEXT("TMap<const FFortAthenaLivingWorldEvent*, FLivingWorldEventRuntimeData>::FindOrAdd"))
        .If(!NativeFindOrCreateEventRuntimeData && StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(!NativeFindOrCreateEventRuntimeData && StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(!NativeFindOrCreateEventRuntimeData && StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto EventRuntimeDataMap = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::EventRuntimeDataMap"))
        .If(NativeFindOrAddEventRuntimeData != nullptr)
        .Optional()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Or()
        .If(NativeFindOrAddEventRuntimeData != nullptr)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Or()
        .If(NativeFindOrAddEventRuntimeData != nullptr)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Disp();

    if (NativeFindOrCreateEventRuntimeData)
        return *NativeFindOrCreateEventRuntimeData(this, Event);

    if (NativeFindOrAddEventRuntimeData && EncounterInstance)
        return *NativeFindOrAddEventRuntimeData(Event, EncounterInstance->GetEventRuntimeDataMap());

    if (NativeFindOrAddEventRuntimeData && EventRuntimeDataMap > 0)
        return *NativeFindOrAddEventRuntimeData(Event, (uint8*)this + EventRuntimeDataMap);

    for (int32 Index = EventRuntimeData.Num() - 1; Index >= 0; Index--)
    {
        if (EventRuntimeData[Index].LivingWorldManager.Get() != this)
            EventRuntimeData.RemoveAt(Index);
        else if (EventRuntimeData[Index].Event == Event)
            return EventRuntimeData[Index].RuntimeData;
    }

    FLivingWorldEventRuntimeDataEntry RuntimeDataEntry;
    RuntimeDataEntry.LivingWorldManager = this;
    RuntimeDataEntry.Event = Event;

    return EventRuntimeData[EventRuntimeData.Add(RuntimeDataEntry)].RuntimeData;
}

UFortAthenaLivingWorldManager::FLivingWorldCategoryRuntimeData* UFortAthenaLivingWorldManager::FindOrCreateCategoryRuntimeData(const FFortAthenaLivingWorldCategory* Category)
{
    static auto NativeFindOrCreateCategoryRuntimeData = Anchor::Optional<FLivingWorldCategoryRuntimeData* (*)(UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldCategory*)>(TEXT("UFortAthenaLivingWorldManager::FindOrCreateCategoryRuntimeData"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("mov rdx, *; mov rcx, *; call 0x*; mov rcx, [rax]; test rcx, rcx")
        .Find("call 0x*")
        .Target()
        .Get();

    return NativeFindOrCreateCategoryRuntimeData ? NativeFindOrCreateCategoryRuntimeData(this, Category) : nullptr;
}

UFortAthenaLivingWorldConfigData* UFortAthenaLivingWorldManager::GetConfig()
{
    static auto CachedConfig = GetClass()->FindPropertyByName(FName(TEXT("CachedConfig")));

    return CachedConfig ? *CachedConfig->ContainerPtrToValuePtr<UFortAthenaLivingWorldConfigData*>(this) : GetLagerConfig().LoadSynchronous();
}

void UFortAthenaLivingWorldManager::AddPointProvider(AActor* PointProvider)
{
    static auto AddPointProviderToEventRuntimeData = Anchor::Optional<void (*)(UFortAthenaLivingWorldManager*, const FScriptInterface*)>(TEXT("UFortAthenaLivingWorldManager::AddPointProviderToEventRuntimeData"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:SetFiltersTags"))
        .Find("lea rdx, [rbp-0x*]; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!HasPointProviderInterfaces())
    {
        GetPointProviders().AddUnique(PointProvider);
        return;
    }

    auto& PointProviders = GetPointProviderInterfaces();
    bool bRegistered = false;

    for (auto& RegisteredPointProvider : PointProviders)
        bRegistered |= RegisteredPointProvider.GetObject() == PointProvider;

    FScriptInterface NewPointProvider;
    NewPointProvider.SetObject(PointProvider);
    NewPointProvider.SetInterface(PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()));

    if (!bRegistered)
    {
        PointProviders.Add(NewPointProvider);

        if (AddPointProviderToEventRuntimeData)
            AddPointProviderToEventRuntimeData(this, &NewPointProvider);
    }

    if (AddPointProviderToEventRuntimeData || FLivingWorldEventInstance::GetEncounterInstanceOffset() <= 0)
        return;

    for (auto EncounterInstance : GetRunningEncounterInstances())
    {
        if (EncounterInstance)
            EncounterInstance->TryAddActivePointProvider(NewPointProvider);
    }
}

void UFortAthenaLivingWorldManager::RemovePointProvider(AActor* PointProvider)
{
    static auto RemovePointProviderFromEventRuntimeData = Anchor::Optional<void (*)(UFortAthenaLivingWorldManager*, const FScriptInterface*)>(TEXT("UFortAthenaLivingWorldManager::RemovePointProviderFromEventRuntimeData"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:SetFiltersTags"))
        .Find("mov rcx, rbx; movdqa [rbp-0x*], xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!HasPointProviderInterfaces())
    {
        GetPointProviders().RemoveSingle(PointProvider);
        return;
    }

    auto& PointProviders = GetPointProviderInterfaces();

    for (int32 Index = 0; Index < PointProviders.Num(); Index++)
    {
        if (PointProviders[Index].GetObject() == PointProvider)
        {
            PointProviders.RemoveAt(Index);
            break;
        }
    }

    FScriptInterface OldPointProvider;
    OldPointProvider.SetObject(PointProvider);
    OldPointProvider.SetInterface(PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()));

    if (RemovePointProviderFromEventRuntimeData)
    {
        RemovePointProviderFromEventRuntimeData(this, &OldPointProvider);
        return;
    }

    if (FLivingWorldEventInstance::GetEncounterInstanceOffset() <= 0)
        return;

    for (auto EncounterInstance : GetRunningEncounterInstances())
    {
        if (EncounterInstance)
            EncounterInstance->RemoveActivePointProvider(OldPointProvider);
    }
}

TSoftObjectPtr<UFortAthenaLivingWorldConfigData>& UFortAthenaLivingWorldManager::GetLagerConfig()
{
    static auto LagerConfig = GetClass()->FindPropertyByName(FName(TEXT("LagerConfig")));

    return *LagerConfig->ContainerPtrToValuePtr<TSoftObjectPtr<UFortAthenaLivingWorldConfigData>>(this);
}

FScalableFloat& UFortAthenaLivingWorldManager::GetLagerEnabled()
{
    static auto LagerEnabled = GetClass()->FindPropertyByName(FName(TEXT("LagerEnabled")));

    return *LagerEnabled->ContainerPtrToValuePtr<FScalableFloat>(this);
}

bool UFortAthenaLivingWorldManager::HasPointProviderInterfaces()
{
    static auto PointProviders = (UArrayProperty*)GetClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    return PointProviders->GetInner()->GetSize() == sizeof(FScriptInterface);
}

TArray<AActor*>& UFortAthenaLivingWorldManager::GetPointProviders()
{
    static auto PointProviders = GetClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    return *PointProviders->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

TArray<FScriptInterface>& UFortAthenaLivingWorldManager::GetPointProviderInterfaces()
{
    static auto PointProviders = GetClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    return *PointProviders->ContainerPtrToValuePtr<TArray<FScriptInterface>>(this);
}

TArray<UClass*>& UFortAthenaLivingWorldManager::GetLoadedSpawnerDataClass()
{
    static auto LoadedSpawnerDataClass = GetClass()->FindPropertyByName(FName(TEXT("LoadedSpawnerDataClass")));

    return *LoadedSpawnerDataClass->ContainerPtrToValuePtr<TArray<UClass*>>(this);
}

AFortGameStateAthena* UFortAthenaLivingWorldManager::GetCachedGameState()
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    if (CachedGameState)
        return *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>(this);

    auto CachedWorld = GetCachedWorld();

    return CachedWorld && *CachedWorld ? Cast<AFortGameStateAthena>((*CachedWorld)->GetGameState()) : nullptr;
}

void UFortAthenaLivingWorldManager::SetCachedGameState(AFortGameStateAthena* GameState)
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    if (CachedGameState)
        *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>(this) = GameState;
}

FFortAthenaLivingWorldPlayerTracker* UFortAthenaLivingWorldManager::GetPlayerTracker()
{
    static auto PlayerTracker = GetClass()->FindPropertyByName(FName(TEXT("PlayerTracker")));

    return PlayerTracker ? PlayerTracker->ContainerPtrToValuePtr<FFortAthenaLivingWorldPlayerTracker>(this) : nullptr;
}

TArray<UFortAthenaLivingWorldEncounterInstance*>& UFortAthenaLivingWorldManager::GetRunningEncounterInstances()
{
    static auto RunningEncounterInstances = GetClass()->FindPropertyByName(FName(TEXT("RunningEncounterInstances")));

    return *RunningEncounterInstances->ContainerPtrToValuePtr<TArray<UFortAthenaLivingWorldEncounterInstance*>>(this);
}

TArray<AActor*>& UFortAthenaLivingWorldManager::GetRuntimePointProviderList()
{
    static auto RuntimePointProviderList = GetClass()->FindPropertyByName(FName(TEXT("RuntimePointProviderList")));

    return *RuntimePointProviderList->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

TRuntimeSizedArray<UFortAthenaLivingWorldManager::FLivingWorldEventInstance>& UFortAthenaLivingWorldManager::GetEventInstances()
{
    return *(TRuntimeSizedArray<FLivingWorldEventInstance>*)((uint8*)this + GetEventInstancesOffset());
}

TArray<UFortAthenaLivingWorldManager::FLivingWorldPendingActorSpawnEventRequest>& UFortAthenaLivingWorldManager::GetPendingActorSpawnEventRequests()
{
    return *(TArray<FLivingWorldPendingActorSpawnEventRequest>*)((uint8*)this + GetPendingActorSpawnEventRequestsOffset());
}

TQueue<UFortAthenaLivingWorldManager::FLivingWorldActorUpdateInfo>* UFortAthenaLivingWorldManager::GetActorInfoUpdateQueue()
{
    static auto ActorInfoUpdateQueue = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::ActorInfoUpdateQueue"))
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("lea rcx, [r13+0x*]; lea rdx, [rbp-0x*]; call 0x*")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("lea rcx, [r14+0x*]; lea rdx, [rbp-0x*]; call 0x*")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("lea rcx, [r15+0x*]; lea rdx, [rbp-0x*]; call 0x*")
        .Optional()
        .Disp();

    if (ActorInfoUpdateQueue > 0)
        return (TQueue<FLivingWorldActorUpdateInfo>*)((uint8*)this + ActorInfoUpdateQueue);

    auto Config = GetConfig();

    if (!Config || !Config->GetMaxActorUpdatesPerTick())
        return nullptr;

    static TQueue<FLivingWorldActorUpdateInfo> OwnActorInfoUpdateQueue;

    return &OwnActorInfoUpdateQueue;
}

float& UFortAthenaLivingWorldManager::GetNextEventGenerationTime()
{
    static auto NextEventGenerationTime = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::NextEventGenerationTime"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("mov rcx, *; call 0x*; mov rbx, [rsp+0x*]")
        .Find("call 0x*")
        .Target()
        .Find("movss [*+0x*], xmm0; call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call [rax+0x*]")
        .Disp();

    return *(float*)((uint8*)this + NextEventGenerationTime);
}

FGameplayTagContainer& UFortAthenaLivingWorldManager::GetPlaylistContextTags()
{
    static auto PlaylistContextTags = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::PlaylistContextTags"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*")
        .Disp();

    return *(FGameplayTagContainer*)((uint8*)this + PlaylistContextTags);
}

UWorld** UFortAthenaLivingWorldManager::GetCachedWorld()
{
    static auto CachedWorld = GetClass()->FindPropertyByName(FName(TEXT("CachedWorld")));

    return CachedWorld ? CachedWorld->ContainerPtrToValuePtr<UWorld*>(this) : nullptr;
}

AActor* UFortAthenaLivingWorldManager::GetEQSHelper()
{
    static auto EQSHelper = GetClass()->FindPropertyByName(FName(TEXT("EQSHelper")));

    return EQSHelper ? *EQSHelper->ContainerPtrToValuePtr<AActor*>(this) : nullptr;
}

FFortAthenaLivingWorldDensityGrid* UFortAthenaLivingWorldManager::GetActorDensityGrid()
{
    static auto ActorDensityGrid = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::ActorDensityGrid"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, r12; call 0x*")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea r14, [rcx+0x*]; movaps [rsp+0x*], xmm6; mov rbp, rcx")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rbx, [rcx+0x*]; movaps [rax-0x*], xmm6; mov rsi, rcx")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rbx, [rcx+0x*]; movaps [rax-0x*], xmm6; mov r14, rcx")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rbx, [rsi+0x*]; mov rdx, rdi; mov rcx, rbx; call 0x*")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea r12, [r*+0x*]; mov rdx, r*; mov rcx, r12; call 0x*")
        .Optional()
        .Disp();

    return ActorDensityGrid > 0 ? (FFortAthenaLivingWorldDensityGrid*)((uint8*)this + ActorDensityGrid) : nullptr;
}

FFortAthenaLivingWorldDensityGrid* UFortAthenaLivingWorldManager::GetTagDensityGrid(const FGameplayTag& Tag)
{
    static auto NativeGetTagDensityGrid = Anchor::Optional<FFortAthenaLivingWorldDensityGrid* (*)(UFortAthenaLivingWorldManager*, const TGameValue<FGameplayTag>*)>(TEXT("UFortAthenaLivingWorldManager::GetTagDensityGrid"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov rdx, rsi; mov rcx, rbx; call 0x*; test rax, rax; jz 0x*; movss xmm2, dword ptr [rsi+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov rdx, rbx; mov rcx, rdi; call 0x*; test rax, rax; jz 0x*; movss xmm2, dword ptr [rbx+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov rdx, r14; mov rcx, rbx; call 0x*; test rax, rax; jz 0x*; movss xmm2, dword ptr [r14+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Pattern("48 83 EC 28 8B 02 4C 8D 99 ? ? ? ? 4C 8B CA C1 E8 10 0F B7 12 8B CA 44 69 C0 01 00 08 00 C1 E1 10 44 03 C2 C1 EA 04 44 03 C1 49 8B CB 41 03 D0 4D 8B C1 E8 ? ? ? ? 83 F8 FF 74 0E 48 98 48 69 D0 ? ? ? ? 49 03 13")
        .Get();

    if (NativeGetTagDensityGrid)
    {
        const TGameValue<FGameplayTag> GameTag(Tag);

        return NativeGetTagDensityGrid(this, &GameTag);
    }

    static auto TagDensityGrids = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::TagDensityGrids"))
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("lea rbx, [r13+0x*]; mov r12, rax; xor r15d, r15d")
        .Optional()
        .Disp();

    static auto FindId = Anchor::Optional<int32* (*)(void*, int32*, const FGameplayTag*)>(TEXT("TMap<FGameplayTag, FFortAthenaLivingWorldDensityGrid>::FindId"))
        .If(TagDensityGrids > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("mov r8, rdi; lea rdx, [rbp-0x*]; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto ElementSize = Anchor::Offset(TEXT("sizeof(TSetElement<TPair<FGameplayTag, FFortAthenaLivingWorldDensityGrid>>)"))
        .If(TagDensityGrids > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("mov r8, rdi; lea rdx, [rbp-0x*]; mov rcx, rbx; call 0x*")
        .Find("imul rcx, rax, 0x*")
        .Optional()
        .Imm();

    static auto Grid = Anchor::Offset(TEXT("TPair<FGameplayTag, FFortAthenaLivingWorldDensityGrid>::Value"))
        .If(TagDensityGrids > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Find("mov r8, rdi; lea rdx, [rbp-0x*]; mov rcx, rbx; call 0x*")
        .Find("lea rax, [rcx+0x*]; neg rcx")
        .Optional()
        .Disp();

    if (TagDensityGrids <= 0 || !FindId || ElementSize <= 0 || Grid <= 0)
        return nullptr;

    auto TagDensityGridMap = (uint8*)this + TagDensityGrids;
    int32 ElementId = INDEX_NONE;

    FindId(TagDensityGridMap, &ElementId, &Tag);

    return ElementId != INDEX_NONE ? (FFortAthenaLivingWorldDensityGrid*)(*(uint8**)TagDensityGridMap + ElementId * ElementSize + Grid) : nullptr;
}

TArray<AActor*>* UFortAthenaLivingWorldManager::GetSpawnedPawns()
{
    static auto SpawnedPawns = GetClass()->FindPropertyByName(FName(TEXT("SpawnedPawns")));

    return SpawnedPawns ? SpawnedPawns->ContainerPtrToValuePtr<TArray<AActor*>>(this) : nullptr;
}

float UFortAthenaLivingWorldManager::GetActorDensity(const FVector& Location, float Radius)
{
    static auto GetActorDensity = Anchor::Optional<float (*)(UFortAthenaLivingWorldManager*, const TGameValue<FVector>&, float)>(TEXT("UFortAthenaLivingWorldManager::GetActorDensity"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("xorps xmm2, xmm2; movsd xmm0, qword ptr [rax]; lea rdx, [rsp+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movaps xmm2, xmm0; lea rdx, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movss xmm2, dword ptr [r14+0x*]; lea rdx, [rsp+0x*]; mov rcx, rdi; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movss xmm2, dword ptr [r12+0x*]; lea rdx, [rbp-0x*]; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movss xmm2, dword ptr [r15+0x*]; lea rdx, [rbp-0x*]; mov rcx, rdi; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto GetGridActorDensity = Anchor::Optional<int32 (*)(FFortAthenaLivingWorldDensityGrid*, const TGameValue<FVector>&, float)>(TEXT("FFortAthenaLivingWorldDensityGrid::GetActorDensity"))
        .If(!GetActorDensity)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("lea rcx, [rdi+0x*]; movaps xmm2, xmm0; lea rdx, [rsp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto GetGridDensity = Anchor::Optional<float (*)(FFortAthenaLivingWorldDensityGrid*, const TGameValue<FVector>&, float)>(TEXT("FFortAthenaLivingWorldDensityGrid::GetDensity"))
        .If(!GetActorDensity && !GetGridActorDensity)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("lea rcx, [rbx+0x*]; mov eax, [rax+0x8]; lea rdx, [rsp+0x*]; movsd [rsp+0x*], xmm0; mov [rsp+0x*], eax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(!GetActorDensity && !GetGridActorDensity)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movss xmm2, dword ptr [r15+0x*]; lea rcx, [rdi+0x*]; lea rdx, [rbp-0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (GetActorDensity)
        return GetActorDensity(this, Location, Radius);

    auto ActorDensityGrid = GetActorDensityGrid();

    if (GetGridDensity && ActorDensityGrid)
        return GetGridDensity(ActorDensityGrid, Location, Radius);

    return GetGridActorDensity && ActorDensityGrid ? (float)GetGridActorDensity(ActorDensityGrid, Location, Radius) : 0.0f;
}

float UFortAthenaLivingWorldManager::GetMaxActorDensity()
{
    auto Config = GetConfig();
    auto MaxActorDensity = Config ? Config->GetMaxActorDensity() : nullptr;

    return MaxActorDensity ? MaxActorDensity->GetValueAtLevel(0.0f) : 0.0f;
}

void UFortAthenaLivingWorldManager::ClusterPoints(const TArray<FVector>& Points, TArray<FVector>& ClusteredPoints)
{
    auto Config = GetConfig();

    if (!Config)
        return;

    ClusteredPoints.Empty();

    if (Points.Num() <= 0)
        return;

    auto PointClusterSize = Config->GetPointClusterSize();
    const float HalfClusterSize = (PointClusterSize ? PointClusterSize->GetValueAtLevel(0.0f) : 0.0f) * 0.5f;

    if (HalfClusterSize <= 0.0f)
    {
        ClusteredPoints.Append(Points);
        return;
    }

    for (auto& Point : Points)
    {
        bool bInCluster = false;

        for (auto& ClusteredPoint : ClusteredPoints)
        {
            if (Point.X > ClusteredPoint.X - HalfClusterSize && Point.X < ClusteredPoint.X + HalfClusterSize && Point.Y > ClusteredPoint.Y - HalfClusterSize && Point.Y < ClusteredPoint.Y + HalfClusterSize)
            {
                bInCluster = true;
                break;
            }
        }

        if (!bInCluster)
            ClusteredPoints.Add(Point);
    }
}

int32 UFortAthenaLivingWorldManager::GetActorDensityBudget(const FVector& Location, float Radius)
{
    auto Config = GetConfig();
    auto MaxActorDensity = Config ? Config->GetMaxActorDensity() : nullptr;

    return MaxActorDensity ? FMath::Max(0, (int32)(MaxActorDensity->GetValueAtLevel(0.0f) - GetActorDensity(Location, Radius))) : 0;
}

static_assert(sizeof(UFortAthenaLivingWorldManager::FEventRuntimeSpawnData::FActorDescription) == 0x88);
static_assert(sizeof(UFortAthenaLivingWorldManager::FEventRuntimeSpawnData) == 0x48);
static_assert(sizeof(UFortAthenaLivingWorldManager::FEventDataRuntime) == 0x98);
static_assert(sizeof(UFortAthenaLivingWorldManager::FEventRuntime) == 0x48);
static_assert(sizeof(UFortAthenaLivingWorldManager::FCategoryRuntime) == 0x38);
static_assert(sizeof(UFortAthenaLivingWorldManager::FEventRequest) == 0x48);
static_assert(sizeof(UFortAthenaLivingWorldManager::FPendingActorSpawnEventRequest::FActorInfo) == 0x70);
static_assert(sizeof(UFortAthenaLivingWorldManager::FPendingActorSpawnEventRequest) == 0x90);

bool UFortAthenaLivingWorldManager::HasEventRuntimes()
{
    static auto bHasEventRuntimes = Anchor::Exists()
        .If(StaticClass() && StaticClass()->FindPropertyByName(FName(TEXT("GlobalConditionContainer"))) && GetPendingActorSpawnEventRequestsOffset() == StaticClass()->FindPropertyByName(FName(TEXT("GlobalConditionContainer")))->GetOffset_ForInternal() + 0xD8)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Is("push rbx; sub rsp, 0x20; add rcx, 0x*; mov [rsp+0x38], rdx; lea rbx, [r8+0x*]")
        .Get();

    return bHasEventRuntimes;
}

UFortAthenaLivingWorldManager::FEventRuntime* UFortAthenaLivingWorldManager::FindEventRuntime(const FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEncounterInstance* EncounterInstance)
{
    static auto FindEventRuntime = Anchor::Function<FEventRuntime* (*)(UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*, UFortAthenaLivingWorldEncounterInstance*)>(TEXT("UFortAthenaLivingWorldManager::FindEventRuntime"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Get();

    return FindEventRuntime(this, Event, EncounterInstance);
}

UFortAthenaLivingWorldManager::FCategoryRuntime* UFortAthenaLivingWorldManager::FindCategoryRuntime(const FFortAthenaLivingWorldCategory* Category)
{
    static auto FindCategoryRuntime = Anchor::Function<FCategoryRuntime* (*)(UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldCategory*)>(TEXT("UFortAthenaLivingWorldManager::FindCategoryRuntime"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 4)
        .Target()
        .Get();

    return FindCategoryRuntime(this, Category);
}

TMap<const FFortAthenaLivingWorldCategory*, UFortAthenaLivingWorldManager::FCategoryRuntime>& UFortAthenaLivingWorldManager::GetCategoryRuntimes()
{
    static auto CategoryRuntimes = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::CategoryRuntimes"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 4)
        .Target()
        .Find("lea r11, [rcx+0x*]")
        .Disp();

    return *(TMap<const FFortAthenaLivingWorldCategory*, FCategoryRuntime>*)((uint8*)this + CategoryRuntimes);
}

TMap<const FFortAthenaLivingWorldEvent*, UFortAthenaLivingWorldManager::FEventRuntime>& UFortAthenaLivingWorldManager::GetEventRuntimes()
{
    static auto EventRuntimes = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::EventRuntimes"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("add rcx, 0x*")
        .Imm();

    return *(TMap<const FFortAthenaLivingWorldEvent*, FEventRuntime>*)((uint8*)this + EventRuntimes);
}

TArray<UFortAthenaLivingWorldManager::FEventRequest>& UFortAthenaLivingWorldManager::GetPendingEventRequests()
{
    static auto GlobalConditionContainer = StaticClass()->FindPropertyByName(FName(TEXT("GlobalConditionContainer")));

    return *(TArray<FEventRequest>*)((uint8*)this + GlobalConditionContainer->GetOffset_ForInternal() + GlobalConditionContainer->GetSize());
}

TArray<UFortAthenaLivingWorldManager::FPendingActorSpawnEventRequest>& UFortAthenaLivingWorldManager::GetPendingActorSpawnRequests()
{
    return *(TArray<FPendingActorSpawnEventRequest>*)((uint8*)this + GetPendingActorSpawnEventRequestsOffset());
}

int32 UFortAthenaLivingWorldManager::GetTotalActorCount()
{
    int32 TotalActorCount = 0;

    for (auto& CategoryRuntime : GetCategoryRuntimes())
    {
        if (CategoryRuntime.Key && !((FFortAthenaLivingWorldCategory*)CategoryRuntime.Key)->GetbExcludeFromGlobalAIBudget())
            TotalActorCount += CategoryRuntime.Value.CurrentSpawnedActorCount;
    }

    return TotalActorCount;
}

void UFortAthenaLivingWorldManager::TickEventRuntimes(float DeltaTime)
{
    static auto NextEventGenerationTimeOffset = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::NextEventGenerationTime"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:RequestEventGeneration"))
        .Find("movss [rcx+0x*], xmm0")
        .Disp();

    auto World = GetWorld();
    auto Config = GetConfig();

    if (!World || !Config || NextEventGenerationTimeOffset <= 0)
        return;

    auto& NextEventGenerationTime = *(float*)((uint8*)this + NextEventGenerationTimeOffset);

    if (NextEventGenerationTime < 0.0f)
        return;

    if (World->GetTimeSeconds() >= NextEventGenerationTime)
    {
        NextEventGenerationTime = World->GetTimeSeconds() + Config->GetMinimumEventGenerationInterval().GetValueAtLevel(0.0f);

        RegisterStaticPointProviders();
        GenerateEventsFromRuntimes();
    }

    ProcessPendingEventRequests();
}

void UFortAthenaLivingWorldManager::GenerateEventsFromRuntimes()
{
    static auto ConstructConditionContext = Anchor::Function<void* (*)(void*)>(TEXT("FWorldConditionContextData::FWorldConditionContextData"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 8)
        .Target()
        .Get();

    auto Config = GetConfig();

    if (!Config)
        return;

    if ((int32)Config->GetMaxActorCount().GetValueAtLevel(0.0f) - GetTotalActorCount() <= 0)
        return;

    TArray<FFortAthenaLivingWorldCategory*> Categories;

    for (auto& CategoryRuntime : GetCategoryRuntimes())
        Categories.Add((FFortAthenaLivingWorldCategory*)CategoryRuntime.Key);

    Sort(Categories.GetData(), Categories.Num(), [](FFortAthenaLivingWorldCategory& A, FFortAthenaLivingWorldCategory& B) { return A.GetPriority() > B.GetPriority(); });

    struct
    {
        void* Schema;
        TArray<uint8> Views;
    } ConditionContext{};

    ConstructConditionContext(&ConditionContext);

    for (auto Category : Categories)
        ProcessCategory(Category, &ConditionContext);
}

void UFortAthenaLivingWorldManager::ProcessCategory(FFortAthenaLivingWorldCategory* Category, void* ConditionContext)
{
    static auto IsConditionTrue = Anchor::Function<bool (*)(void*, int32, void*)>(TEXT("FFortAthenaLivingWorldConditionContainer::IsTrue"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 9)
        .Target()
        .Get();

    static auto GetCategoryActorBudget = Anchor::Function<int32 (*)(UFortAthenaLivingWorldManager*, FFortAthenaLivingWorldCategory*, FCategoryRuntime*)>(TEXT("UFortAthenaLivingWorldManager::GetCategoryActorBudget"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 12)
        .Target()
        .Get();

    static auto GetCategoryEventInstanceBudget = Anchor::Function<int32 (*)(FFortAthenaLivingWorldCategory*, FCategoryRuntime*)>(TEXT("FFortAthenaLivingWorldCategory::GetEventInstanceBudget"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 14)
        .Target()
        .Get();

    static auto GlobalConditionContainer = StaticClass()->FindPropertyByName(FName(TEXT("GlobalConditionContainer")));

    if (!Category || !GetConfig())
        return;

    auto CategoryRuntime = FindCategoryRuntime(Category);

    if (!CategoryRuntime || CategoryRuntime->Events.Num() == 0)
        return;

    if (!IsConditionTrue(GlobalConditionContainer->ContainerPtrToValuePtr<void>(this), CategoryRuntime->ConditionHandle, ConditionContext))
        return;

    const int32 ActorBudget = GetCategoryActorBudget(this, Category, CategoryRuntime);
    const int32 EventInstanceBudget = GetCategoryEventInstanceBudget(Category, CategoryRuntime);

    GenerateEventRequests(nullptr, Category, CategoryRuntime->Events, ActorBudget, EventInstanceBudget, ConditionContext);
}

void UFortAthenaLivingWorldManager::ComputeActiveEvents(const TArray<FFortAthenaLivingWorldEvent*>& Events, UFortAthenaLivingWorldEncounterInstance* EncounterInstance, void* ConditionContext, TArray<FFortAthenaLivingWorldEvent*>& OutActiveEvents, float& OutTotalWeight)
{
    static auto GetGamePhaseStepAndSafeZonePhase = Anchor::Function<void (*)(UFortAthenaLivingWorldManager*, uint8*, int32*)>(TEXT("UFortAthenaLivingWorldManager::GetGamePhaseStepAndSafeZonePhase"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 5)
        .Target()
        .Get();

    static auto ShouldSkipPhaseValidation = Anchor::Function<bool (*)(UFortAthenaLivingWorldManager*)>(TEXT("UFortAthenaLivingWorldManager::ShouldSkipPhaseValidation"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 6)
        .Target()
        .Get();

    static auto IsEventActive = Anchor::Function<bool (*)(FFortAthenaLivingWorldEvent*, UFortAthenaLivingWorldManager*, FGameplayTagContainer*, bool, uint8, int32)>(TEXT("FFortAthenaLivingWorldEvent::IsActive"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 7)
        .Target()
        .Get();

    static auto IsConditionTrue = Anchor::Function<bool (*)(void*, int32, void*)>(TEXT("FFortAthenaLivingWorldConditionContainer::IsTrue"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 9)
        .Target()
        .Get();

    static auto GetEventInstanceBudget = Anchor::Function<int32 (*)(FFortAthenaLivingWorldEvent*, FEventRuntime*, int32)>(TEXT("FFortAthenaLivingWorldEvent::GetEventInstanceBudget"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 13)
        .Target()
        .Get();

    static auto PlaylistContextTags = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::PlaylistContextTags"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea r8, [rbx+0x*]; mov r9b, al")
        .Disp();

    static auto GlobalConditionContainer = StaticClass()->FindPropertyByName(FName(TEXT("GlobalConditionContainer")));
    static auto ActivationMinDelay = FFortAthenaLivingWorldEvent::StaticStruct()->FindPropertyByName(FName(TEXT("ActivationMinDelay")));
    static auto ActivationMaxDelay = FFortAthenaLivingWorldEvent::StaticStruct()->FindPropertyByName(FName(TEXT("ActivationMaxDelay")));
    static auto DeactivationMinDelay = FFortAthenaLivingWorldEvent::StaticStruct()->FindPropertyByName(FName(TEXT("DeactivationMinDelay")));
    static auto DeactivationMaxDelay = FFortAthenaLivingWorldEvent::StaticStruct()->FindPropertyByName(FName(TEXT("DeactivationMaxDelay")));

    const float Now = GetWorld()->GetTimeSeconds();
    const bool bSkipPhaseValidation = ShouldSkipPhaseValidation(this);
    uint8 GamePhaseStep = 0;
    int32 SafeZonePhase = 0;

    GetGamePhaseStepAndSafeZonePhase(this, &GamePhaseStep, &SafeZonePhase);

    for (auto Event : Events)
    {
        if (!Event || !IsEventActive(Event, this, (FGameplayTagContainer*)((uint8*)this + PlaylistContextTags), bSkipPhaseValidation, GamePhaseStep, SafeZonePhase))
            continue;

        auto EventRuntime = FindEventRuntime(Event, EncounterInstance);

        if (!EventRuntime || EventRuntime->DeactivationReason != 0)
            continue;

        if (!IsConditionTrue(GlobalConditionContainer->ContainerPtrToValuePtr<void>(this), EventRuntime->ConditionHandle, ConditionContext))
            continue;

        if (EventRuntime->ActivationTime == -1.0f)
        {
            const float MinDelay = *ActivationMinDelay->ContainerPtrToValuePtr<float>(Event);
            const float MaxDelay = *ActivationMaxDelay->ContainerPtrToValuePtr<float>(Event);

            EventRuntime->ActivationTime = Now + MinDelay + FMath::FRand() * (MaxDelay - MinDelay);
        }

        if (Now < EventRuntime->ActivationTime)
            continue;

        if (EventRuntime->DeactivationTime == -1.0f)
        {
            const float MinDelay = *DeactivationMinDelay->ContainerPtrToValuePtr<float>(Event);
            const float MaxDelay = *DeactivationMaxDelay->ContainerPtrToValuePtr<float>(Event);

            EventRuntime->DeactivationTime = MinDelay >= 0.0f ? Now + MinDelay + FMath::FRand() * (MaxDelay - MinDelay) : FLT_MAX;
        }

        if (Now > EventRuntime->DeactivationTime)
            continue;

        if (GetEventInstanceBudget(Event, EventRuntime, SafeZonePhase) == 0)
            continue;

        OutActiveEvents.Add(Event);
        OutTotalWeight += Event->GetWeight();
    }
}

bool UFortAthenaLivingWorldManager::IsActivePointProvider(const FScriptInterface& PointProvider, const FGameplayTagQuery& TagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter)
{
    auto PointProviderInterface = (IFortAthenaLivingWorldPointProviderInterface*)PointProvider.GetInterface();

    if (!PointProvider.GetObject() || !PointProviderInterface)
        return false;

    if (!TagQuery.IsEmpty())
    {
        FGameplayTagContainer FiltersTags;

        PointProviderInterface->GetFiltersTags(FiltersTags);

        if (!TagQuery.Matches(FiltersTags))
            return false;
    }

    return (*(bool (***)(IFortAthenaLivingWorldPointProviderInterface*, const FFortAthenaLivingWorldPointProviderFilterRules*))PointProviderInterface)[0x50 / sizeof(void*)](PointProviderInterface, &PointFilter);
}

FScriptInterface UFortAthenaLivingWorldManager::GetRandomPointProviderFromTagQuery(const TArray<FScriptInterface>& PointProviders, const FGameplayTagQuery& TagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, int32 StartIndex)
{
    const int32 FirstIndex = FMath::Max(StartIndex, 0);
    const int32 Count = PointProviders.Num() - FirstIndex;
    TArray<int32> Shuffle;

    if (Count > 0)
    {
        Shuffle.AddUninitialized(Count);

        for (int32 Index = 0; Index < Count; Index++)
        {
            const int32 SwapIndex = FMath::Min(FMath::TruncToInt(FMath::FRand() * (Index + 1)), Index);

            if (SwapIndex != Index)
                Shuffle[Index] = Shuffle[SwapIndex];

            Shuffle[SwapIndex] = Index + FirstIndex;
        }
    }

    for (int32 Index : Shuffle)
    {
        if (IsActivePointProvider(PointProviders[Index], TagQuery, PointFilter))
            return PointProviders[Index];
    }

    return FScriptInterface();
}

FScriptInterface UFortAthenaLivingWorldManager::GetRandomPointProviderFromEventDataRuntime(FEventDataRuntime* EventDataRuntime, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter, int32 StartIndex)
{
    static FGameplayTagQuery EmptyTagQuery;

    const FGameplayTagQuery* TagQuery = &EmptyTagQuery;
    const float Roll = FMath::FRand() * EventDataRuntime->TotalFilterWeight;
    float Weight = 0.0f;

    for (int32 Index = 0; Index < EventDataRuntime->FilterEntries.Num(); Index++)
    {
        auto FilterEntry = (FPointProviderFilterEntry*)(EventDataRuntime->FilterEntries.GetData() + Index * FPointProviderFilterEntry::GetTypeSize());

        Weight += FilterEntry->GetWeight().GetValueAtLevel(0.0f);

        if (Roll <= Weight)
        {
            TagQuery = &FilterEntry->GetProviderFiltersTagQuery();
            break;
        }
    }

    return GetRandomPointProviderFromTagQuery(EventDataRuntime->PointProviders, *TagQuery, PointFilter, StartIndex);
}

void UFortAthenaLivingWorldManager::GenerateEventRequests(UFortAthenaLivingWorldEncounterInstance* EncounterInstance, FFortAthenaLivingWorldCategory* Category, const TArray<FFortAthenaLivingWorldEvent*>& Events, int32 ActorBudget, int32 EventInstanceBudget, void* ConditionContext)
{
    static auto GetGamePhaseStepAndSafeZonePhase = Anchor::Function<void (*)(UFortAthenaLivingWorldManager*, uint8*, int32*)>(TEXT("UFortAthenaLivingWorldManager::GetGamePhaseStepAndSafeZonePhase"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 5)
        .Target()
        .Get();

    static auto GetEventActorBudget = Anchor::Function<int32 (*)(FFortAthenaLivingWorldEvent*, FEventRuntime*, int32)>(TEXT("FFortAthenaLivingWorldEvent::GetEventActorBudget"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 11)
        .Target()
        .Get();

    static auto GetEventInstanceBudget = Anchor::Function<int32 (*)(FFortAthenaLivingWorldEvent*, FEventRuntime*, int32)>(TEXT("FFortAthenaLivingWorldEvent::GetEventInstanceBudget"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 13)
        .Target()
        .Get();

    static auto ConstructPointProviderFilterRules = Anchor::Function<void* (*)(FFortAthenaLivingWorldPointProviderFilterRules*, UFortAthenaLivingWorldManager*, FFortAthenaLivingWorldEvent*, UObject*, int32, float, float)>(TEXT("FFortAthenaLivingWorldPointProviderFilterRules::FFortAthenaLivingWorldPointProviderFilterRules"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 15)
        .Target()
        .Get();

    static auto ComputeEventRuntimeSpawnData = Anchor::Function<void (*)(UFortAthenaLivingWorldManager*, void*, FEventRuntime*, void*, int32, bool, void*, TArray<FEventRuntimeSpawnData*>*, TArray<int32>*, float*)>(TEXT("UFortAthenaLivingWorldManager::ComputeEventRuntimeSpawnData"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 19)
        .Target()
        .Get();

    static auto NextEventRequestID = 0;

    if (ActorBudget == 0 || EventInstanceBudget == 0 || !Category)
        return;

    auto CategoryRuntime = FindCategoryRuntime(Category);
    TArray<FFortAthenaLivingWorldEvent*> ActiveEvents;
    float TotalWeight = 0.0f;

    ComputeActiveEvents(Events, EncounterInstance, ConditionContext, ActiveEvents, TotalWeight);

    if (!CategoryRuntime || TotalWeight <= 0.0f || ActiveEvents.Num() == 0)
        return;

    uint8 GamePhaseStep = 0;
    int32 SafeZonePhase = 0;
    int32 SpawnedActorCount = 0;

    GetGamePhaseStepAndSafeZonePhase(this, &GamePhaseStep, &SafeZonePhase);

    for (int32 Attempt = 0; Attempt < ActorBudget && SpawnedActorCount < ActorBudget && ActiveEvents.Num() != 0 && EventInstanceBudget != 0 && TotalWeight > 0.0f; Attempt++)
    {
        const float EventRoll = FMath::FRand() * TotalWeight;
        FFortAthenaLivingWorldEvent* Event = nullptr;
        float EventWeight = 0.0f;

        for (auto ActiveEvent : ActiveEvents)
        {
            if (!ActiveEvent || ActiveEvent->GetWeight() <= 0.0f)
                continue;

            EventWeight += ActiveEvent->GetWeight();

            if (EventRoll <= EventWeight)
            {
                Event = ActiveEvent;
                break;
            }
        }

        if (!Event)
            continue;

        auto EventRuntime = FindEventRuntime(Event, EncounterInstance);
        int32 RemainingActorBudget = ActorBudget - SpawnedActorCount;
        const int32 EventActorBudget = GetEventActorBudget(Event, EventRuntime, SafeZonePhase);

        if (EventActorBudget < RemainingActorBudget)
            RemainingActorBudget = EventActorBudget;

        TArray<FEventRuntimeSpawnData*> SpawnDatas;
        TArray<int32> StartIndices;
        float TotalSpawnDataWeight = 0.0f;

        ComputeEventRuntimeSpawnData(this, nullptr, EventRuntime, nullptr, RemainingActorBudget, false, ConditionContext, &SpawnDatas, &StartIndices, &TotalSpawnDataWeight);

        FEventRuntimeSpawnData* SpawnData = nullptr;
        int32 SpawnDataIndex = INDEX_NONE;

        if (SpawnDatas.Num() != 0 && TotalSpawnDataWeight > 0.0f)
        {
            const float SpawnDataRoll = FMath::FRand() * TotalSpawnDataWeight;
            float SpawnDataWeight = 0.0f;

            for (int32 Index = 0; Index < SpawnDatas.Num(); Index++)
            {
                SpawnDataWeight += SpawnDatas[Index]->Weight;

                if (SpawnDataWeight >= SpawnDataRoll)
                {
                    SpawnData = SpawnDatas[Index];
                    SpawnDataIndex = Index;
                    break;
                }
            }
        }

        FScriptInterface PointProvider;
        int32 ActorCount = 0;

        if (SpawnData)
        {
            const int32 ActorCountRange = SpawnData->MaxActorCount - SpawnData->MinActorCount;

            ActorCount = SpawnData->MinActorCount + (ActorCountRange + 1 > 0 ? FMath::Min(FMath::TruncToInt(FMath::FRand() * (ActorCountRange + 1)), ActorCountRange) : 0);
            ActorCount = FMath::Min(FMath::Min(ActorCount, SpawnData->ActorDescriptions.Num()), RemainingActorBudget);

            auto EventDataRuntime = &EventRuntime->EventDataRuntimes[SpawnData->EventDataRuntimeIndex];
            float DensityForActors = 0.0f;

            for (int32 Index = 0; Index < ActorCount; Index++)
                DensityForActors += SpawnData->ActorDescriptions[Index].DensityValue;

            alignas(8) uint8 PointFilter[0x40] = {};

            ConstructPointProviderFilterRules((FFortAthenaLivingWorldPointProviderFilterRules*)PointFilter, this, Event, EventDataRuntime->EventData.Get(), ActorCount, DensityForActors, SpawnData->DensityRange);

            PointProvider = GetRandomPointProviderFromEventDataRuntime(EventDataRuntime, *(FFortAthenaLivingWorldPointProviderFilterRules*)PointFilter, StartIndices.IsValidIndex(SpawnDataIndex) ? StartIndices[SpawnDataIndex] : INDEX_NONE);
        }

        if (!SpawnData)
        {
            ActiveEvents.RemoveSingle(Event);
            TotalWeight -= Event->GetWeight();
            continue;
        }

        const bool bHasPointProvider = PointProvider.GetObject() && PointProvider.GetInterface();

        EventRuntime->GameSpawnedActorCount += ActorCount;
        EventRuntime->CurrentSpawnedActorCount += ActorCount;
        EventRuntime->GameEventInstanceCount++;
        EventRuntime->CurrentEventInstanceCount++;
        CategoryRuntime->GameSpawnedActorCount += ActorCount;
        CategoryRuntime->CurrentSpawnedActorCount += ActorCount;
        CategoryRuntime->GameEventInstanceCount++;
        CategoryRuntime->CurrentEventInstanceCount++;

        struct
        {
            UObject* EventData;
            int32 ActorCount;
        } RequestedEvent{ EventRuntime->EventDataRuntimes[SpawnData->EventDataRuntimeIndex].EventData.Get(), ActorCount };

        if (bHasPointProvider)
            (*(void (***)(void*, void*))PointProvider.GetInterface())[0x38 / sizeof(void*)](PointProvider.GetInterface(), &RequestedEvent);

        auto& EventRequest = GetPendingEventRequests()[GetPendingEventRequests().AddUninitialized()];

        FMemory::Memzero(&EventRequest, sizeof(FEventRequest));

        EventRequest.Category = Category;
        EventRequest.Event = Event;
        EventRequest.SpawnData = SpawnData;
        EventRequest.EncounterInstance = EncounterInstance;
        EventRequest.PointProvider = PointProvider;
        EventRequest.ActorCount = ActorCount;
        EventRequest.RequestID = NextEventRequestID++;
        EventRequest.bHasPointProvider = bHasPointProvider;

        SpawnedActorCount += ActorCount;
        EventInstanceBudget--;

        if (GetEventInstanceBudget(Event, EventRuntime, SafeZonePhase) == 0)
        {
            ActiveEvents.RemoveSingle(Event);
            TotalWeight -= Event->GetWeight();
        }
    }
}

void UFortAthenaLivingWorldManager::ReturnSpawnedCountToRuntime(const FEventRequest& EventRequest)
{
    if (auto EventRuntime = FindEventRuntime(EventRequest.Event, EventRequest.EncounterInstance))
    {
        EventRuntime->GameSpawnedActorCount -= EventRequest.ActorCount;
        EventRuntime->CurrentSpawnedActorCount -= EventRequest.ActorCount;
        EventRuntime->GameEventInstanceCount--;
        EventRuntime->CurrentEventInstanceCount--;
    }

    if (auto CategoryRuntime = FindCategoryRuntime(EventRequest.Category))
    {
        CategoryRuntime->GameSpawnedActorCount -= EventRequest.ActorCount;
        CategoryRuntime->CurrentSpawnedActorCount -= EventRequest.ActorCount;
        CategoryRuntime->GameEventInstanceCount--;
        CategoryRuntime->CurrentEventInstanceCount--;
    }
}

void UFortAthenaLivingWorldManager::CancelEventRequest(const FEventRequest& EventRequest)
{
    auto PointProviderInterface = EventRequest.PointProvider.GetObject() ? EventRequest.PointProvider.GetInterface() : nullptr;

    if (PointProviderInterface)
    {
        auto EventRuntime = FindEventRuntime(EventRequest.Event, EventRequest.EncounterInstance);
        auto EventData = EventRuntime && EventRequest.SpawnData ? EventRuntime->EventDataRuntimes[EventRequest.SpawnData->EventDataRuntimeIndex].EventData.Get() : nullptr;

        for (int32 Index = 0; Index < EventRequest.ActorCount; Index++)
            (*(void (***)(void*, UObject**))PointProviderInterface)[0x28 / sizeof(void*)](PointProviderInterface, &EventData);

        struct
        {
            UObject* EventData;
            bool bCancelled;
        } FinishedEvent{ EventData, true };

        (*(void (***)(void*, void*))PointProviderInterface)[0x40 / sizeof(void*)](PointProviderInterface, &FinishedEvent);
    }

    ReturnSpawnedCountToRuntime(EventRequest);
}

bool UFortAthenaLivingWorldManager::SelectDataToSpawn(FEventRuntimeSpawnData::FActorDescription& ActorDescription, FEventDataRuntime* EventDataRuntime, UClass*& OutSpawnerDataClass, UClass*& OutActorClass)
{
    static auto SpawnerDataLimiter = StaticClass()->FindPropertyByName(FName(TEXT("SpawnerDataLimiter")));
    static auto SpawnedSingleUseSpawnerData = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldSpawnerDataLimiter")))->FindPropertyByName(FName(TEXT("SpawnedSingleUseSpawnerData")));
    static auto SpawnedConcurrentSingleUseSpawnerData = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldSpawnerDataLimiter")))->FindPropertyByName(FName(TEXT("SpawnedConcurrentSingleUseSpawnerData")));

    if (ActorDescription.ActorClass && ActorDescription.ActorClass->IsChildOf(AActor::StaticClass()))
    {
        OutActorClass = ActorDescription.ActorClass;
        return true;
    }

    auto Limiter = SpawnerDataLimiter->ContainerPtrToValuePtr<uint8>(this);
    auto& SingleUse = *SpawnedSingleUseSpawnerData->ContainerPtrToValuePtr<TArray<UClass*>>(Limiter);
    auto& ConcurrentSingleUse = *SpawnedConcurrentSingleUseSpawnerData->ContainerPtrToValuePtr<TArray<UClass*>>(Limiter);
    auto SpawnerDataFilter = EventDataRuntime->SpawnerDataFilterClass ? EventDataRuntime->SpawnerDataFilter : nullptr;
    TArray<UClass*> Candidates;

    for (auto SpawnerDataClass : ActorDescription.SpawnerDataClasses)
    {
        if (SingleUse.Contains(SpawnerDataClass) || ConcurrentSingleUse.Contains(SpawnerDataClass))
            continue;

        if (SpawnerDataFilter && !(*(bool (***)(void*, UClass**))SpawnerDataFilter)[0x10 / sizeof(void*)](SpawnerDataFilter, &SpawnerDataClass))
            continue;

        Candidates.Add(SpawnerDataClass);
    }

    if (Candidates.Num() == 0)
        return false;

    OutSpawnerDataClass = Candidates[FMath::Min(FMath::TruncToInt(FMath::FRand() * Candidates.Num()), Candidates.Num() - 1)];

    return true;
}

int32 UFortAthenaLivingWorldManager::RequestSpawnToAISpawnerSystem(UAthenaAISpawner* AISpawner, const FVector& Location, const FRotator& Rotation, UClass* SpawnerDataClass, UClass* ActorClass, AActor*& OutSpawnedActor, bool& bOutCompleted)
{
    static auto NextActorRequestID = -2;

    auto SpawnerData = SpawnerDataClass ? SpawnerDataClass->GetDefaultObject() : nullptr;

    if (Cast<UFortAthenaAISpawnerData>(SpawnerData))
    {
        auto ComponentList = AISpawner ? UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerDataClass, this) : nullptr;

        if (!ComponentList)
        {
            bOutCompleted = true;
            return NextActorRequestID--;
        }

        return AISpawner->RequestSpawn(ComponentList, FTransform(FRotator{ 0.0f, Rotation.Yaw, 0.0f }, Location, FVector(1.0f, 1.0f, 1.0f)), TDelegate<void(AActor*, int32)>::CreateUObject(this, &UFortAthenaLivingWorldManager::OnActorSpawned));
    }

    if (auto ActorSpawnerData = Cast<UFortAthenaActorSpawnerData>(SpawnerData))
        ActorClass = ActorSpawnerData->GetActorClass();

    bOutCompleted = true;

    if (!ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
        return NextActorRequestID--;

    const FTransform SpawnTransform(Rotation, Location, FVector(1.0f, 1.0f, 1.0f));

    OutSpawnedActor = GetWorld()->SpawnActor(ActorClass, &SpawnTransform);

    return NextActorRequestID--;
}

void UFortAthenaLivingWorldManager::ProcessPendingEventRequests()
{
    auto Config = GetConfig();

    if (!Config)
        return;

    auto& PendingEventRequests = GetPendingEventRequests();
    const int32 Count = FMath::Min(PendingEventRequests.Num(), (int32)Config->GetMaxEventSpawnPerTick().GetValueAtLevel(0.0f));

    if (Count <= 0)
        return;

    TArray<FEventRequest> EventRequests;

    for (int32 Index = 0; Index < Count; Index++)
        EventRequests.Add(PendingEventRequests[Index]);

    PendingEventRequests.RemoveAt(0, Count);

    for (auto& EventRequest : EventRequests)
        ProcessEventRequest(EventRequest);
}

void UFortAthenaLivingWorldManager::ProcessEventRequest(FEventRequest& EventRequest)
{
    static auto ConstructPointProviderFilterRules = Anchor::Function<void* (*)(FFortAthenaLivingWorldPointProviderFilterRules*, UFortAthenaLivingWorldManager*, FFortAthenaLivingWorldEvent*, UObject*, int32, float, float)>(TEXT("FFortAthenaLivingWorldPointProviderFilterRules::FFortAthenaLivingWorldPointProviderFilterRules"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov r8, rbx; mov rdx, rax; mov rcx, r15; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 15)
        .Target()
        .Get();

    auto SpawnData = EventRequest.SpawnData;
    auto Event = EventRequest.Event;
    auto World = GetWorld();

    if (!SpawnData || !Event || !World)
        return;

    auto PointProviderInterface = EventRequest.PointProvider.GetObject() ? EventRequest.PointProvider.GetInterface() : nullptr;

    if (!PointProviderInterface)
    {
        CancelEventRequest(EventRequest);
        return;
    }

    auto EventRuntime = FindEventRuntime(Event, EventRequest.EncounterInstance);
    auto EventDataRuntime = EventRuntime ? &EventRuntime->EventDataRuntimes[SpawnData->EventDataRuntimeIndex] : nullptr;
    auto AISystem = Cast<UAthenaAISystem>(World->GetAISystem());
    auto AISpawner = AISystem ? AISystem->GetAISpawner() : nullptr;

    if (!EventDataRuntime)
    {
        CancelEventRequest(EventRequest);
        return;
    }

    struct FSpawnInfo
    {
        UClass* SpawnerDataClass = nullptr;
        UClass* ActorClass = nullptr;
        FVector Location = FVector(ForceInit);
        FRotator Rotation{ 0.0f, 0.0f, 0.0f };
    };

    TArray<FSpawnInfo> SpawnInfos;
    bool bHasDefaultLocation = false;
    FVector DefaultLocation(ForceInit);
    FRotator DefaultRotation{ 0.0f, 0.0f, 0.0f };

    for (int32 Index = 0; Index < EventRequest.ActorCount && Index < SpawnData->ActorDescriptions.Num(); Index++)
    {
        auto& ActorDescription = SpawnData->ActorDescriptions[Index];
        FSpawnInfo SpawnInfo;

        if (!SelectDataToSpawn(ActorDescription, EventDataRuntime, SpawnInfo.SpawnerDataClass, SpawnInfo.ActorClass))
            continue;

        if (ActorDescription.bSpawnAroundDefaultPoint && bHasDefaultLocation)
        {
            SpawnInfo.Location = DefaultLocation;
            SpawnInfo.Rotation = DefaultRotation;
        }
        else
        {
            const int32 RemainingActorCount = EventRequest.ActorCount - Index;
            float DensityForActors = 0.0f;

            for (int32 ActorIndex = Index; ActorIndex < Index + RemainingActorCount && ActorIndex < SpawnData->ActorDescriptions.Num(); ActorIndex++)
                DensityForActors += SpawnData->ActorDescriptions[ActorIndex].DensityValue;

            alignas(8) uint8 PointFilter[0x40] = {};
            TGameValue<FVector> GameLocation(SpawnInfo.Location);
            TGameValue<FRotator> GameRotation(SpawnInfo.Rotation);

            ConstructPointProviderFilterRules((FFortAthenaLivingWorldPointProviderFilterRules*)PointFilter, this, Event, EventDataRuntime->EventData.Get(), RemainingActorCount, DensityForActors, SpawnData->DensityRange);

            if (!(*(bool (***)(void*, void*, TGameValue<FVector>*, TGameValue<FRotator>*, void*))PointProviderInterface)[0x10 / sizeof(void*)](PointProviderInterface, PointFilter, &GameLocation, &GameRotation, nullptr))
                continue;

            SpawnInfo.Location = GameLocation.Get();
            SpawnInfo.Rotation = GameRotation.Get();

            if (!bHasDefaultLocation)
            {
                DefaultLocation = SpawnInfo.Location;
                DefaultRotation = SpawnInfo.Rotation;
                bHasDefaultLocation = true;
            }
        }

        SpawnInfos.Add(SpawnInfo);
    }

    if (SpawnInfos.Num() != EventRequest.ActorCount || SpawnInfos.Num() == 0)
    {
        CancelEventRequest(EventRequest);
        return;
    }

    auto& PendingActorSpawnRequests = GetPendingActorSpawnRequests();
    auto& PendingRequest = PendingActorSpawnRequests[PendingActorSpawnRequests.AddUninitialized()];

    FMemory::Memzero(&PendingRequest, sizeof(FPendingActorSpawnEventRequest));

    PendingRequest.Category = EventRequest.Category;
    PendingRequest.Event = Event;
    PendingRequest.SpawnData = SpawnData;
    PendingRequest.EncounterInstance = EventRequest.EncounterInstance;
    PendingRequest.PointProvider = EventRequest.PointProvider;
    PendingRequest.ActorCount = EventRequest.ActorCount;
    PendingRequest.RequestID = EventRequest.RequestID;
    PendingRequest.Location[0] = DefaultLocation.X;
    PendingRequest.Location[1] = DefaultLocation.Y;
    PendingRequest.Location[2] = DefaultLocation.Z;
    PendingRequest.Rotation[0] = DefaultRotation.Pitch;
    PendingRequest.Rotation[1] = DefaultRotation.Yaw;
    PendingRequest.Rotation[2] = DefaultRotation.Roll;

    TArray<AActor*> SpawnedActors;
    TArray<int32> RequestIDs;
    TArray<bool> CompletedRequests;

    for (int32 Index = 0; Index < SpawnInfos.Num(); Index++)
    {
        auto& SpawnInfo = SpawnInfos[Index];
        AActor* SpawnedActor = nullptr;
        bool bCompleted = false;
        const int32 RequestID = RequestSpawnToAISpawnerSystem(AISpawner, SpawnInfo.Location, SpawnInfo.Rotation, SpawnInfo.SpawnerDataClass, SpawnInfo.ActorClass, SpawnedActor, bCompleted);
        auto& ActorInfo = PendingRequest.ActorInfos[PendingRequest.ActorInfos.AddUninitialized()];

        FMemory::Memzero(&ActorInfo, sizeof(FPendingActorSpawnEventRequest::FActorInfo));

        ActorInfo.Location[0] = SpawnInfo.Location.X;
        ActorInfo.Location[1] = SpawnInfo.Location.Y;
        ActorInfo.Location[2] = SpawnInfo.Location.Z;
        ActorInfo.SpawnerDataClass = SpawnInfo.SpawnerDataClass;
        ActorInfo.ActorClass = SpawnInfo.ActorClass;
        ActorInfo.RequestID = RequestID;
        ActorInfo.PreRegistrationHandle = INDEX_NONE;
        ActorInfo.Index = Index;

        SpawnedActors.Add(SpawnedActor);
        RequestIDs.Add(RequestID);
        CompletedRequests.Add(bCompleted);

        if (SpawnInfo.SpawnerDataClass)
        {
            static auto SpawnerDataLimiter = StaticClass()->FindPropertyByName(FName(TEXT("SpawnerDataLimiter")));
            static auto SingleUseSpawnerDataQuery = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldSpawnerDataLimiter")))->FindPropertyByName(FName(TEXT("SingleUseSpawnerDataQuery")));
            static auto ConcurrentSingleUseSpawnerDataQuery = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldSpawnerDataLimiter")))->FindPropertyByName(FName(TEXT("ConcurrentSingleUseSpawnerDataQuery")));
            static auto SpawnedSingleUseSpawnerData = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldSpawnerDataLimiter")))->FindPropertyByName(FName(TEXT("SpawnedSingleUseSpawnerData")));
            static auto SpawnedConcurrentSingleUseSpawnerData = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldSpawnerDataLimiter")))->FindPropertyByName(FName(TEXT("SpawnedConcurrentSingleUseSpawnerData")));

            auto Limiter = SpawnerDataLimiter->ContainerPtrToValuePtr<uint8>(this);
            auto AISpawnerData = Cast<UFortAthenaAISpawnerData>(SpawnInfo.SpawnerDataClass->GetDefaultObject());
            auto ActorSpawnerData = Cast<UFortAthenaActorSpawnerData>(SpawnInfo.SpawnerDataClass->GetDefaultObject());
            auto DescriptorTag = AISpawnerData ? &AISpawnerData->GetDescriptorTag() : ActorSpawnerData ? &ActorSpawnerData->GetDescriptorTag() : nullptr;

            if (DescriptorTag && !SingleUseSpawnerDataQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(Limiter)->IsEmpty() && SingleUseSpawnerDataQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(Limiter)->Matches(*DescriptorTag))
                SpawnedSingleUseSpawnerData->ContainerPtrToValuePtr<TArray<UClass*>>(Limiter)->AddUnique(SpawnInfo.SpawnerDataClass);

            if (DescriptorTag && !ConcurrentSingleUseSpawnerDataQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(Limiter)->IsEmpty() && ConcurrentSingleUseSpawnerDataQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(Limiter)->Matches(*DescriptorTag))
                SpawnedConcurrentSingleUseSpawnerData->ContainerPtrToValuePtr<TArray<UClass*>>(Limiter)->AddUnique(SpawnInfo.SpawnerDataClass);
        }
    }

    for (int32 Index = 0; Index < RequestIDs.Num(); Index++)
    {
        if (CompletedRequests[Index])
            OnActorSpawned(SpawnedActors[Index], RequestIDs[Index]);
    }
}

void UFortAthenaLivingWorldManager::AddPointProviderToEventRuntime(const FScriptInterface& PointProvider)
{
    auto PointProviderInterface = (IFortAthenaLivingWorldPointProviderInterface*)PointProvider.GetInterface();

    if (!PointProvider.GetObject() || !PointProviderInterface)
        return;

    FGameplayTagContainer FiltersTags;

    PointProviderInterface->GetFiltersTags(FiltersTags);

    for (auto& EventRuntime : GetEventRuntimes())
    {
        for (auto& EventDataRuntime : ((FEventRuntime&)EventRuntime.Value).EventDataRuntimes)
        {
            if (EventDataRuntime.ProviderFilters.IsEmpty() || EventDataRuntime.ProviderFilters.Matches(FiltersTags))
                EventDataRuntime.PointProviders.Add(PointProvider);
        }
    }
}

void UFortAthenaLivingWorldManager::RemovePointProviderFromEventRuntime(const FScriptInterface& PointProvider)
{
    for (auto& EventRuntime : GetEventRuntimes())
    {
        for (auto& EventDataRuntime : ((FEventRuntime&)EventRuntime.Value).EventDataRuntimes)
        {
            for (int32 Index = EventDataRuntime.PointProviders.Num() - 1; Index >= 0; Index--)
            {
                if (EventDataRuntime.PointProviders[Index].GetObject() == PointProvider.GetObject())
                    EventDataRuntime.PointProviders.RemoveAt(Index);
            }
        }
    }
}

void UFortAthenaLivingWorldManager::EnablePointProvider(const FScriptInterface& PointProvider)
{
    static auto PointProvidersProperty = StaticClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    if (!PointProvider.GetObject() || !PointProvider.GetInterface() || !IsLivingWorldEnabled() || !GetOwner()->HasAuthority())
        return;

    auto& PointProviders = *PointProvidersProperty->ContainerPtrToValuePtr<TArray<FScriptInterface>>(this);

    for (auto& RegisteredPointProvider : PointProviders)
    {
        if (RegisteredPointProvider.GetObject() == PointProvider.GetObject())
            return;
    }

    PointProviders.Add(PointProvider);

    AddPointProviderToEventRuntime(PointProvider);
}

void UFortAthenaLivingWorldManager::DisablePointProvider(const FScriptInterface& PointProvider)
{
    static auto PointProvidersProperty = StaticClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    if (!PointProvider.GetObject() || !PointProvider.GetInterface() || !GetOwner()->HasAuthority())
        return;

    auto& PointProviders = *PointProvidersProperty->ContainerPtrToValuePtr<TArray<FScriptInterface>>(this);

    for (int32 Index = PointProviders.Num() - 1; Index >= 0; Index--)
    {
        if (PointProviders[Index].GetObject() == PointProvider.GetObject())
            PointProviders.RemoveAt(Index);
    }

    RemovePointProviderFromEventRuntime(PointProvider);

    auto& PendingEventRequests = GetPendingEventRequests();

    for (int32 Index = PendingEventRequests.Num() - 1; Index >= 0; Index--)
    {
        if (PendingEventRequests[Index].PointProvider.GetObject() == PointProvider.GetObject())
            PendingEventRequests.RemoveAt(Index);
    }
}

void UFortAthenaLivingWorldManager::RegisterStaticPointProviders()
{
    static auto StaticPointProviderClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldStaticPointProvider"));
    static auto StaticCachedLivingWorldManager = StaticPointProviderClass ? StaticPointProviderClass->FindPropertyByName(FName(TEXT("CachedLivingWorldManager"))) : nullptr;
    static auto bStaticStartEnabled = StaticPointProviderClass ? (UBoolProperty*)StaticPointProviderClass->FindPropertyByName(FName(TEXT("bStartEnabled"))) : nullptr;
    static auto bVolumeStartEnabled = AFortAthenaLivingWorldVolume::StaticClass() ? (UBoolProperty*)AFortAthenaLivingWorldVolume::StaticClass()->FindPropertyByName(FName(TEXT("bStartEnabled"))) : nullptr;
    static auto PatrolCachedLivingWorldManager = AFortAthenaPatrolPathPointProvider::StaticClass() ? AFortAthenaPatrolPathPointProvider::StaticClass()->FindPropertyByName(FName(TEXT("CachedLivingWorldManager"))) : nullptr;

    if (!GetOwner()->HasAuthority())
        return;

    TArray<AActor*> PointProviders;

    if (StaticCachedLivingWorldManager)
        UGameplayStatics::GetAllActorsOfClass(GetOwner(), StaticPointProviderClass, PointProviders);

    for (auto PointProvider : PointProviders)
    {
        if (*StaticCachedLivingWorldManager->ContainerPtrToValuePtr<UFortAthenaLivingWorldManager*>(PointProvider) == this)
            continue;

        if (bStaticStartEnabled && !bStaticStartEnabled->GetPropertyValue_InContainer(PointProvider))
        {
            *StaticCachedLivingWorldManager->ContainerPtrToValuePtr<UFortAthenaLivingWorldManager*>(PointProvider) = this;
            continue;
        }

        LivingWorldManagerRegisterPointProvider(PointProvider);
    }

    TArray<AActor*> Volumes;

    if (AFortAthenaLivingWorldVolume::StaticClass())
        UGameplayStatics::GetAllActorsOfClass(GetOwner(), AFortAthenaLivingWorldVolume::StaticClass(), Volumes);

    for (auto Actor : Volumes)
    {
        auto Volume = (AFortAthenaLivingWorldVolume*)Actor;
        auto CachedLivingWorldManager = Volume->GetCachedLivingWorldManager();

        if (!CachedLivingWorldManager || *CachedLivingWorldManager == this)
            continue;

        if (bVolumeStartEnabled && !bVolumeStartEnabled->GetPropertyValue_InContainer(Volume))
        {
            *CachedLivingWorldManager = this;
            continue;
        }

        Volume->EnablePointProvider();
    }

    TArray<AActor*> PatrolPathPointProviders;

    if (PatrolCachedLivingWorldManager)
        UGameplayStatics::GetAllActorsOfClass(GetOwner(), AFortAthenaPatrolPathPointProvider::StaticClass(), PatrolPathPointProviders);

    for (auto Actor : PatrolPathPointProviders)
    {
        auto PatrolPathPointProvider = (AFortAthenaPatrolPathPointProvider*)Actor;

        if (*PatrolCachedLivingWorldManager->ContainerPtrToValuePtr<UFortAthenaLivingWorldManager*>(PatrolPathPointProvider) == this)
            continue;

        if (!PatrolPathPointProvider->GetbStartEnabled() || !PatrolPathPointProvider->GetAssociatedPatrolPath())
            continue;

        LivingWorldManagerRegisterPointProvider(PatrolPathPointProvider);
    }
}
