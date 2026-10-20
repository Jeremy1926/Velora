#include "pch.h"
#include "NavigationSystem.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

void UNavigationSystemV1::ProcessRegistrationCandidates()
{
    static auto ProcessRegistrationCandidates = Anchor::VirtualFunction<void (*)(UNavigationSystemV1*)>(TEXT("UNavigationSystemV1::ProcessRegistrationCandidates"))
        .String(TEXT("Auto generating navigation bounds: %s"))
        .Find("call [rax+0x*]")
        .Get();

    ProcessRegistrationCandidates(this);
}

void UNavigationSystemV1::SpawnMissingNavigationData()
{
    static auto SpawnMissingNavigationData = Anchor::VirtualFunction<void (*)(UNavigationSystemV1*)>(TEXT("UNavigationSystemV1::SpawnMissingNavigationData"))
        .String(TEXT("Auto generating navigation bounds: %s"))
        .Find("mov rcx, *; call 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    SpawnMissingNavigationData(this);
}

bool UNavigationSystemV1::IsNavigationBeingBuilt(UObject* WorldContextObject)
{
    static auto IsNavigationBeingBuilt = Anchor::Function<bool (*)(UObject*)>(TEXT("UNavigationSystemV1::IsNavigationBeingBuilt"))
        .Native(TEXT("/Script/NavigationSystem.NavigationSystemV1:IsNavigationBeingBuilt"))
        .Get();

    return IsNavigationBeingBuilt(WorldContextObject);
}

ANavigationData* UNavigationSystemV1::GetMainNavData() const
{
    static auto MainNavData = GetClass()->FindPropertyByName(FName(TEXT("MainNavData")));

    return *MainNavData->ContainerPtrToValuePtr<ANavigationData*>(this);
}

TArray<ANavigationData*>& UNavigationSystemV1::GetNavDataSet()
{
    static auto NavDataSet = GetClass()->FindPropertyByName(FName(TEXT("NavDataSet")));

    return *NavDataSet->ContainerPtrToValuePtr<TArray<ANavigationData*>>(this);
}

TArray<ANavigationData*>& UNavigationSystemV1::GetNavDataRegistrationQueue()
{
    static auto NavDataRegistrationQueue = GetClass()->FindPropertyByName(FName(TEXT("NavDataRegistrationQueue")));

    return *NavDataRegistrationQueue->ContainerPtrToValuePtr<TArray<ANavigationData*>>(this);
}
