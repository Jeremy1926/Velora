#include "pch.h"
#include "NavigationSystem.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

void UNavigationSystemV1::ProcessRegistrationCandidates()
{
    static auto ProcessRegistrationCandidates = Anchor::Slot(TEXT("UNavigationSystemV1::ProcessRegistrationCandidates"))
        .String(TEXT("Auto generating navigation bounds: %s"))
        .Find("call [rax+0x*]")
        .Get();

    Scanner::Virtual<void (*)(UNavigationSystemV1*)>(this, ProcessRegistrationCandidates)(this);
}

void UNavigationSystemV1::SpawnMissingNavigationData()
{
    static auto SpawnMissingNavigationData = Anchor::Slot(TEXT("UNavigationSystemV1::SpawnMissingNavigationData"))
        .String(TEXT("Auto generating navigation bounds: %s"))
        .Find("mov rcx, *; call 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    Scanner::Virtual<void (*)(UNavigationSystemV1*)>(this, SpawnMissingNavigationData)(this);
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
